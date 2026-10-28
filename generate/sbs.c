#include "../../SupSyBuildSystem/sbs.h"
#define SL_STRIP_PREFIX
#define SL_IMPLEMENTATION
#include <SL/sl.h>

#define ROOT "/home/supsy/Coding/SupSy/supsy.dev/"



typedef struct file_loc
{
    const char *source_path;
    usize p, l, c;
} file_loc;

#define info(loc, fmt, ...)  (fprintf(stdout, "%s:%02zu:%02zu - [INFO]  "fmt"\n", (loc).source_path, (loc).l + 0, (loc).c + 1, ##__VA_ARGS__))
#define error(loc, fmt, ...) (fprintf(stderr, "%s:%02zu:%02zu - [ERROR] "fmt"\n", (loc).source_path, (loc).l + 0, (loc).c + 1, ##__VA_ARGS__), exit(1))



#pragma region MARKDOWN
typedef enum g_inline_type
{
    INLINE_BOLD   = 0b00000001, // single stars
    INLINE_ITALIC = 0b00000010, // double stars
    INLINE_CODE   = 0b00000100, // monospace font with background
    INLINE_NEWL   = 0b00001000, // newline
    INLINE_LINK   = 0b00010000, // link to external element or element id
    INLINE_IMAGE  = 0b00100000, // inline image
} g_inline_type;

typedef enum g_element_type
{
    ELEM_NONE = 0,
    ELEM_BREAK,
    ELEM_TEXT,
    ELEM_PARAGRAPH,
    ELEM_TITLE,
    ELEM_LIST,
    ELEM_LIST_ITEM,
    ELEM_CODE,
    ELEM_BOX,
} g_element_type;

typedef struct g_inline  g_inline;
typedef struct g_list    g_list;
typedef struct g_title   g_title;
typedef struct g_para    g_para;
typedef struct g_code    g_code;
typedef struct g_box     g_box;
typedef struct g_elem    g_elem;

DEF_ARRAY(g_inline);
DEF_ARRAY(g_list);
DEF_ARRAY(g_elem);

struct g_inline
{
    file_loc loc;
    g_inline_type type;
    char *str, *link;
};

struct g_title
{
    usize depth;
    array(g_inline) text;
};

struct g_para
{
    array(g_inline) text;
};

struct g_list
{
    bool is_ordered;
    int indent;
    array(g_elem) items;
};

struct g_code
{
    char *language;
    array(charp) lines;
};

struct g_box
{
    usize depth;
    array(g_elem) items;
};

struct g_elem
{
    file_loc loc;
    g_element_type type;
    union {
        g_inline in;
        g_title  ti;
        g_para   pa;
        g_list   li;
        g_code   co;
        g_box    bx;
    };
};
#define g_elem_(type_, loc_, ...) ((g_elem){.type = type_, .loc = loc_, ##__VA_ARGS__})

void print_elem(g_elem *elem, usize indent)
{
    switch (elem->type) {
        case ELEM_BREAK: info(elem->loc, "%*s<BREAK>", (int)indent * 4, ""); break;
        case ELEM_TEXT: {
            const char *tag = NULL;
            if (elem->in.type & INLINE_BOLD)   tag = tag ? strf("%s|bold", tag)   : "bold";
            if (elem->in.type & INLINE_ITALIC) tag = tag ? strf("%s|italic", tag) : "italic";
            if (elem->in.type & INLINE_CODE)   tag = tag ? strf("%s|code", tag)   : "code";
            if (elem->in.type & INLINE_LINK)   tag = tag ? strf("%s|link", tag)   : "link";
            if (elem->in.type & INLINE_IMAGE)  tag = tag ? strf("%s|img", tag)    : "img";
            
            if      (elem->in.type == 0)         info(elem->in.loc, "%*s\"%s\"", (int)indent * 4, "", elem->in.str);
            else if (elem->in.type == INLINE_NEWL) info(elem->in.loc, "%*s<br>", (int)indent * 4, "");
            else                                 info(elem->in.loc, "%*s<%s> \"%s\" </%s>", (int)indent * 4, "", tag, elem->in.str, tag);
        } break;
        case ELEM_TITLE: {
            char tag[32] = "################################";
            info(elem->loc, "%*s%.*s {", (int)indent * 4, "", (int)elem->ti.depth, tag);
            aforeach(te, elem->ti.text)
            {
                g_elem as_el = { .type = ELEM_TEXT, .in = *te }; 
                print_elem(&as_el, indent + 1);
            }
            info(elem->loc, "%*s}", (int)indent * 4, "");
        } break;
        case ELEM_PARAGRAPH: {
            info(elem->loc, "%*sp {", (int)indent * 4, "");
            aforeach(te, elem->pa.text)
            {
                g_elem as_el = { .type = ELEM_TEXT, .loc = te->loc, .in = *te }; 
                print_elem(&as_el, indent + 1);
            }
            info(elem->loc, "%*s}", (int)indent * 4, "");
        } break;
        case ELEM_LIST: {
            info(elem->loc, "%*slist {", (int)indent * 4, "");
            aforeach(el, elem->li.items) print_elem(el, indent + 1);
            info(elem->loc, "%*s}", (int)indent * 4, "");
        } break;
        case ELEM_CODE: {
            info(elem->loc, "%*scode(%s) {", (int)indent * 4, "", elem->co.language);
            aforeach(line, elem->co.lines) info(elem->loc, "%*s%s", (int)indent * 4, "", *line);
            info(elem->loc, "%*s}", (int)indent * 4, "");
        } break;
        case ELEM_BOX: {
            info(elem->loc, "%*sbox {", (int)indent * 4, "");
            aforeach(el, elem->bx.items) print_elem(el, indent + 1);
            info(elem->loc, "%*s}", (int)indent * 4, "");
        } break;
        default: break;
    }
}



typedef struct md_reader {
    FILE *f;
    file_loc loc;
    char line[2048];
    int indent;

    array(g_list) list_stack;
    bool reuse_line;
} md_reader;
#define md_at(md, ...) ((md)->line[(md)->loc.c + (0, ##__VA_ARGS__)])
#define md_mv(md, n) ((md)->loc.c += n)

char *readline(md_reader *md)
{
    md->loc.c = 0;
   
    if (md->reuse_line) md->reuse_line = false;
    else {
        if (!fgets(md->line, sizeof(md->line), md->f)) return NULL;
        md->loc.l++;
    }

    int cur_indent = 0;
    while (md->line[md->loc.c] == ' ') md->loc.c++, cur_indent++;
    usize line_len = strlen(md->line);
    if (md->line[line_len - 1] == '\n') md->line[line_len - 1] = '\0';

    md->indent = cur_indent;
    // info(md->loc, "indent = %d, line = \"%.15s\" {...}", cur_indent, md->line);
    return md->line;
}

g_inline parse_md_inline(md_reader *md, g_inline_type *type)
{   
    g_inline text = { .loc = md->loc, .type = *type, .str = NULL };
    if (strcmp(&md_at(md), "  ") == 0)
    {
        text.type = INLINE_NEWL;
        md_mv(md, 2);
        return text;
    }

    const bool is_code   = (*type & INLINE_CODE)   == INLINE_CODE;
    const bool is_italic = (*type & INLINE_ITALIC) == INLINE_ITALIC;
    const bool is_bold   = (*type & INLINE_BOLD)   == INLINE_BOLD;
    
    // info(md->loc, "parse_md_text(is_bold=%d, is_italic=%d, is_code=%d) = \"%s\"", is_bold, is_italic, is_code, md->line + md->loc.c);
    while (md_at(md))
    {
        if (!is_code && !is_italic && md_at(md) == '*' && md_at(md, 1) == '*')
        {
            if (is_bold)
            {
                if (text.loc.c != md->loc.c) text.str = strndup(md->line + text.loc.c, md->loc.c - text.loc.c);
                md_mv(md, 2);
                *type &= ~INLINE_BOLD;
                return text;
            }
            if (!isspace(md_at(md, 2))) {
                md_mv(md, 2);
                *type |= INLINE_BOLD;
                if (text.loc.c == md->loc.c - 2) return parse_md_inline(md, type);

                text.str = strndup(md->line + text.loc.c, md->loc.c - 2 - text.loc.c);
                return text;
            }
        }

        if (!is_code && md_at(md) == '*')
        {
            if (is_italic)
            {
                if (text.loc.c != md->loc.c) text.str = strndup(md->line + text.loc.c, md->loc.c - text.loc.c);
                md_mv(md, 1);
                *type &= ~INLINE_ITALIC;
                return text;
            }
            if (!isspace(md_at(md, 1))) {
                md_mv(md, 1);
                *type |= INLINE_ITALIC;
                if (text.loc.c == md->loc.c - 1) return parse_md_inline(md, type);

                text.str = strndup(md->line + text.loc.c, md->loc.c - 1 - text.loc.c);
                return text;
            }
        }

        if (md_at(md) == '`')
        {
            if (is_code)
            {
                if (text.loc.c != md->loc.c) text.str = strndup(md->line + text.loc.c, md->loc.c - text.loc.c);
                md_mv(md, 1);
                *type &= ~INLINE_CODE;
                return text;
            }
            if (!isspace(md_at(md, 1))) {
                md_mv(md, 1);
                *type |= INLINE_CODE;
                if (text.loc.c == md->loc.c - 1) return parse_md_inline(md, type);

                text.str = strndup(md->line + text.loc.c, md->loc.c - 1 - text.loc.c);
                return text;
            }
        }

        md->loc.c++;
    }

    if (md->loc.c != text.loc.c) {
        if (strcmp(&md_at(md, -2), "  ") == 0) {
            md_mv(md, -2);
            text.str = strndup(md->line + text.loc.c, md->loc.c - text.loc.c);
        }
        else text.str = strndup(md->line + text.loc.c, md->loc.c - text.loc.c);
    }
    return text;
}

bool parse_md_elem(md_reader *md, array(g_elem) *elements, int cindent, int pindent, int box_depth)
{
    usize ordered_label_length = 0;
    switch (md_at(md)) {
        case '\0': arrayAdd(*elements, g_elem_(ELEM_BREAK, md->loc)); break;

        case '#':
        {
            g_elem title = g_elem_(ELEM_TITLE, md->loc);
            while (md_at(md) == '#') md_mv(md, 1), title.ti.depth++;
            if (!isspace(md_at(md))) {
                md->loc = title.loc;
                goto TEXT;
            }
            while (md_at(md) == ' ') md_mv(md, 1);
            g_inline_type t_type = 0;
            while (md_at(md))
            {
                g_inline t = parse_md_inline(md, &t_type);
                if (t.str == NULL) continue;
                arrayAdd(title.ti.text, t);
                if (t.type == INLINE_NEWL) break;
            }
            arrayAdd(*elements, title);
        } break;

        case '`':
        {
            if (!strstart(&md_at(md), "```")) goto TEXT;
            g_elem code = g_elem_(ELEM_CODE, md->loc);

            md_mv(md, 3);
            code.co.language = strdup(&md_at(md));

            while (readline(md))
            {
                if (strstart(&md_at(md), "```"))
                {
                    bool just_spaces = true;
                    for (char *p = &md_at(md, 3); *p; ++p)
                    {
                        if (!isspace(*p)) {
                            just_spaces = false;
                            break;
                        }
                    }
                    if (just_spaces) break;
                }
                arrayAdd(code.co.lines, strdup(md->line));
            }

            arrayAdd(*elements, code);
        } break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': 
        {
            do ++ordered_label_length; while (isdigit(md_at(md, ordered_label_length)));
            if (md_at(md, ordered_label_length) != '.' || !isspace(md_at(md, ordered_label_length + 1))) goto TEXT;
            goto LIST;
        } 
        case '-':
        {
            if (!isspace(md_at(md, 1))) goto TEXT;
        LIST:
            if (md->list_stack.count == 0 || arrayLast(md->list_stack)->indent < cindent)
            {
                // printf("[LIST] Creating new list\n");

                file_loc list_loc = md->loc;
                g_list *list = arrayAdd(md->list_stack, ((g_list) {
                    .indent = cindent, .is_ordered = ordered_label_length > 0
                }));

                while (
                       parse_md_elem(md, &list->items, cindent, list->indent, box_depth)
                    && arrayLast(list->items)->type != ELEM_BREAK
                    && arrayLast(list->items)->loc.c > list->indent
                    && readline(md)
                );
                
                int depth = arrayLast(list->items)->type == ELEM_BREAK ? 
                            (int)arrayLast(list->items)->loc.c : -1;
                            
                if (arrayLast(list->items)->type == ELEM_BREAK) list->items.count--;
                
                arrayAdd(*elements, g_elem_(ELEM_LIST, list_loc, .li = *list));
                md->list_stack.count--;
                
                if (md->list_stack.count && depth < arrayLast(md->list_stack)->indent) 
                {
                    // printf("[LIST] Propagate \"break\"\n");
                    file_loc loc = md->loc;
                    loc.c = depth;
                    arrayAdd(*elements, g_elem_(ELEM_BREAK, loc));
                }
                
                return true;
            }

            // printf("[LIST] Adding new item\n");
            
            arrayAdd(*elements, g_elem_(ELEM_LIST_ITEM, md->loc, .li.is_ordered = ordered_label_length > 0));
            
            md_mv(md, ordered_label_length);
            do md_mv(md, 1); while (md_at(md) == ' ');
            return parse_md_elem(md, &arrayLast(md->list_stack)->items, cindent, arrayLast(md->list_stack)->indent, box_depth);
        } break;

        case '>':
        {
            g_elem box = g_elem_(ELEM_BOX, md->loc);
            md_mv(md, 1);

            if (box_depth) {
                parse_md_elem(md, elements, cindent, pindent, box_depth);
                if (md->line[0] != '>') arrayAdd(*elements, g_elem_(ELEM_BREAK, md->loc));
                return true;
            }
            // usize line_box_depth = md->line[0] == '>';
            // for (char *p = md->line; *p && *(p + 1); ++p) line_box_depth += (*p == ' ' && *(p + 1) == '>');
            // info(md->loc, "cur_box_depth=%zu, box_depth=%zu", line_box_depth);
            
            while (md_at(md) == ' ') md_mv(md, 1);
            while (
                   parse_md_elem(md, &box.bx.items, cindent, pindent, 1)
                && arrayLast(box.bx.items)->type != ELEM_BREAK
                && readline(md)
            );
            
            int depth = arrayLast(box.bx.items)->type == ELEM_BREAK ? 
                   (int)arrayLast(box.bx.items)->loc.c : -1;
                        
            if (arrayLast(box.bx.items)->type == ELEM_BREAK) box.bx.items.count--;
            
            arrayAdd(*elements, box);
            
            file_loc loc = md->loc;
            loc.c = depth;
            arrayAdd(*elements, g_elem_(ELEM_BREAK, loc));
        } break;

        default: TEXT: {
            const bool concat = elements->count && arrayLast(*elements)->type == ELEM_PARAGRAPH;
            
            g_elem paragraph = g_elem_(ELEM_PARAGRAPH, md->loc);
            if (concat) paragraph.pa = arrayLast(*elements)->pa;

            g_inline_type t_type = 0;
            while (md_at(md))
            {
                g_inline t = parse_md_inline(md, &t_type);
                if (t.type == INLINE_NEWL) arrayAdd(paragraph.pa.text, t);
                if (t.str == NULL) continue;


                const char *at = strstr(t.str, "](");
                if ((t.type & INLINE_CODE) == 0 && at) // Most likely a link
                {
                    const char *start = NULL, *end = NULL;
                    for (int i = 0; !start || !end || at[i] || at - i >= t.str; ++i)
                    {
                        if (!start && at - i >= t.str && at[-i] == '[') start = at - i;
                        if (!end && at[i] && at[i] == ')') end = at + i;
                    }
                    if (start && end) // Definitly a link
                    {
                        char *base = t.str;
                        char *label = strndup(start + 1, at - start - 1);
                        char *link  = strndup(at    + 2, end - at - 2);
                        const bool is_img = start[-1] == '!';

                        if (start > base)
                        {
                            t.str  = strndup(base, start - base - is_img);
                            t.link = NULL;
                            arrayAdd(paragraph.pa.text, t);
                        }
                        
                        t.str   = label;
                        t.link  = link;
                        t.type |= is_img ? INLINE_IMAGE : INLINE_LINK;
                        arrayAdd(paragraph.pa.text, t);
                        
                        if (end[1])
                        {
                            t.str   = strdup(end + 1);
                            t.link  = NULL;
                            t.type &= ~(INLINE_LINK | INLINE_IMAGE);
                            arrayAdd(paragraph.pa.text, t);
                        }
                        
                        free(base);
                    }
                }
                else arrayAdd(paragraph.pa.text, t);
            }

            if (concat) arrayLast(*elements)->pa = paragraph.pa;
            else arrayAdd(*elements, paragraph);
        } break;
    }

    return true;
}

typedef struct md_tree
{
    array(g_elem) elements;
    bool is_french;
    const char *title;
    const char *description;
} md_tree;

md_tree parse_md(const char *source_path)
{
    FILE *f = fopen(source_path, "r");
    if (!f) SL_terminate(-1, "Failed to open \"%s\"!", source_path);

    md_tree ret = {0};
    md_reader md = { .f = f, .loc = { .source_path = source_path } };

    if (readline(&md))
    {
        if      (strcmp(md.line, "[FR]") == 0) ret.is_french = true;
        else if (strcmp(md.line, "[EN]") == 0) ret.is_french = false;
        else                                          md.reuse_line = true;
    }
    while (
        readline(&md) &&
        parse_md_elem(&md, &ret.elements, md.indent, -1, 0)
    );
    fclose(f);

    array(char) tmp = {0};
    assert(ret.elements.data[0].type == ELEM_TITLE && "FIRST MUST BE A TITLE");
    aforeach(t, ret.elements.data[0].ti.text) arrayAdd_range(tmp, strlen(t->str), t->str);
    ret.title = strndup(tmp.data, tmp.count);
    
    tmp.count = 0;
    assert(ret.elements.data[1].type == ELEM_PARAGRAPH && "SECOND MUST BE A PARAGRAPH");
    aforeach(t, ret.elements.data[1].pa.text) arrayAdd_range(tmp, strlen(t->str), t->str);
    ret.description = strndup(tmp.data, tmp.count);
    
    return ret;
}
#pragma endregion MARKDOWN



#pragma region GENERATE
bool generate_html_inline(FILE *f, g_inline text)
{
    if (text.type == INLINE_NEWL) 
    {
        fprintf(f, "<br/>");
        return true;
    }
    if (text.type & INLINE_IMAGE)
    {
        fprintf(f, "<img src=\"%s\" alt=\"%s\"/>", text.link, text.str);
        return false;
    }
    
    if (text.type & INLINE_BOLD)   fputs("<b>", f);
    if (text.type & INLINE_ITALIC) fputs("<i>", f);
    if (text.type & INLINE_CODE)   fputs("<code class=\"inline\">", f);
    if (text.type & INLINE_LINK)   fprintf(f, "<a class=\"text-accent underline\" href=\"%s\">", text.link);
    for (char *p = text.str; *p; ++p)
    {
        if      (*p == '&') fputs("&amp", f);
        else if (*p == '<') fputs("&lt", f);
        else                fputc(*p, f);
    }
    if (text.type & INLINE_LINK)   fputs("</a>", f);
    if (text.type & INLINE_CODE)   fputs("</code>", f);
    if (text.type & INLINE_ITALIC) fputs("</i>", f);
    if (text.type & INLINE_BOLD)   fputs("</b>", f);

    return false;
}

void generate_html_elem(FILE *f, usize depth, g_elem elem, bool first, bool direct_section_descendant)
{
    switch (elem.type)
    {
        case ELEM_LIST:
        {
            const char uo = elem.li.is_ordered ? 'o' : 'u';
            fprintf(f, "%*s<%cl>\n", (int)depth * 4, "", uo);
            fprintf(f, "%*s<li>\n", ((int)depth + 1) * 4, "");
            aforeach(el, elem.li.items)
            {
                if (aindex(el) > 0 && el->type == ELEM_LIST_ITEM) 
                {
                    fprintf(f, "%*s</li>\n", ((int)depth + 1) * 4, "");
                    fprintf(f, "%*s<li>\n", ((int)depth + 1) * 4, "");
                }

                generate_html_elem(f, depth + 2, *el, false, false);
            }
            fprintf(f, "%*s</li>\n", ((int)depth + 1) * 4, "");
            fprintf(f, "%*s</%cl>\n", (int)depth * 4, "", uo);
        } break;

        case ELEM_PARAGRAPH:
        {
            fprintf(f, "%*s<p>\n%*s", (int)depth * 4, "", ((int)depth + 1) * 4, "");
            aforeach(text, elem.pa.text)
            {
                if (generate_html_inline(f, *text))
                    fprintf(f, "\n%*s", ((int)depth + 1) * 4, "");
            }
            fprintf(f, "\n%*s</p>\n", (int)depth * 4, "");
        } break;

        case ELEM_TITLE: 
        {
            if (direct_section_descendant && elem.ti.depth == 1 && !first) 
            {
                fprintf(f, "%*s</section>\n\"%*s\"<section>\n", ((int)depth - 1) * 4, "", ((int)depth - 1) * 4, "");
            }

            usize id = 0;
            if (direct_section_descendant)
            {
                while (elem.ti.text.data[0].str[id] && !isspace(elem.ti.text.data[0].str[id])) ++id;
                if (elem.ti.text.data[0].str[id - 1] == '.') --id;

                fprintf(f, 
                    "%*s<h%zu><span class=\"anchor\" id=\"%.*s\"></span>", (int)depth * 4, "", 
                    elem.ti.depth, (int)id, elem.ti.text.data[0].str
                );
            }
            else fprintf(f, "%*s<h%zu>", (int)depth * 4, "", elem.ti.depth);
            aforeach(text, elem.ti.text) generate_html_inline(f, *text);
            
            if (direct_section_descendant)
            {
                fprintf(f, 
                    " <a class=\"text-dim\" href=\"#%.*s\">#</a>",
                    (int)id, elem.ti.text.data[0].str
                );
            }
            fprintf(f, "</h%zu>\n", elem.ti.depth);
        } break;

        case ELEM_CODE:
        {
            fprintf(f, "%*s<pre class=\"code-box\"><code class=\"language-%s\">", (int)depth * 4, "", strlower(elem.co.language));
            aforeach(line, elem.co.lines) {
                if (aindex(line)) fputc('\n', f);
                for (char *p = *line; *p; ++p)
                {
                    if      (*p == '&') fputs("&amp", f);
                    else if (*p == '<') fputs("&lt", f);
                    else                fputc(*p, f);
                }
            }
            fprintf(f, "</code></pre>\n");
            // fprintf(f, "%*s</code></pre>\n", (int)depth * 4, "");
        } break;

        case ELEM_BOX:
        {
            fprintf(f, "%*s<hgroup>\n", (int)depth * 4, "");
            // fprintf(f, "%*s<div class=\"box\">\n", (int)depth * 4, "");
            aforeach(el, elem.bx.items) generate_html_elem(f, depth + 1, *el, false, false);
            fprintf(f, "%*s</hgroup>\n", (int)depth * 4, "");
            // fprintf(f, "%*s</div>\n", (int)depth * 4, "");
        } break;

        default: break;
    }
}



typedef union list_entry {
    struct {
        const char *src;
        const char *date;
        const char *lang;
        const char *title;
        const char *desc;
    };
    const char *data[5];
} list_entry;
DEF_ARRAY(list_entry);
DEF_CMP_FUNC(list_entry, lhs, rhs) { 
    int res;
    if (res = strncmp(lhs->date + 7, rhs->date + 7, 4)) return res;
    if (res = strncmp(lhs->date + 3, rhs->date + 3, 2)) return res;
    if (res = strncmp(lhs->date + 0, rhs->date + 0, 2)) return res;
    return 0;
}

list_entry list_entry_(const char *src, time_t tim, bool is_french, const char *title, const char *desc)
{
    struct tm tm = *localtime(&tim);
    char *date = strf("%02d/%02d/%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

    return (list_entry) {
        .src = src, .date = date, .lang = is_french ? "fr" : "en",
        .title = title, .desc = desc
    };
}

array(list_entry) load_list(const char *path)
{
    array(list_entry) ret = {0};

    FILE *f = fopen(path, "r");
    if (!f) SL_terminate(-1, "Failed to open \"%s\"!", path);
    
    char line[2048];
    list_entry entry = {0};
    usize data = 0;
    while (fgets(line, sizeof(line), f))
    {
        entry.data[data++] = strndup(line, strlen(line) - 1);
        if (data == 5) {
            data = 0;
            arrayAdd(ret, entry);
            if (!fgets(line, sizeof(line), f)) break;
        }
    }

    fclose(f);
    return ret;
}
void save_list(array(list_entry) le, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f) SL_terminate(-1, "Failed to open \"%s\"!", path);

    arraySort(le, list_entry_cmp_inv);
    aforeach(e, le)
    {
        fputs(e->src,   f); fputc('\n', f);
        fputs(e->date,  f); fputc('\n', f);
        fputs(e->lang,  f); fputc('\n', f);
        fputs(e->title, f); fputc('\n', f);
        fputs(e->desc,  f); fputc('\n', f);
        if (e + 1 < __e_MAX__) fputc('\n', f);
    }

    fclose(f);
}
void update_list(const char *path, list_entry new_entry)
{
    array(list_entry) entries = load_list(path);
    aforeach(e, entries)
    {
        if (strcmp(e->src, new_entry.src) == 0)
        {
            arrayRemove(entries, aindex(e));
            break;
        }
    }
    arrayAdd(entries, new_entry);
    save_list(entries, path);
}





void generate_blog(const char *output_path, md_tree md)
{
    char *default_blog = readEntireFile(ROOT"generate/default_blog.html", NULL);
    char *insert = strstr(default_blog, "<!-- <INSERT> -->");

    int indent = 0;
    while (insert - indent >= default_blog && insert[-indent] != '\n') ++indent;
    insert[1 - indent] = '\0';

    const char *first_half  = default_blog;
    const char *second_half = insert + sizeof("<!-- <INSERT> -->"); 

    FILE *f = fopen(output_path, "w");
    if (!f) SL_terminate(-1, "Failed to open \"%s\"!", output_path);
    
    fprintf(f, first_half, md.is_french ? "fr" : "en", md.title);
    aforeach(el, md.elements) generate_html_elem(f, indent / 4, *el, aindex(el) == 0, true);
    fputs(second_half, f);

    fclose(f);
}
#pragma endregion GENERATE

int main(int argc, char **argv)
{
    SBS_rebuild(argc, argv, "-I../../SupSyLibraries/include/");

    if (argc > 1 && strcmp(argv[1], "blog") == 0)
    {
        const char *blog_name = argc > 2 ? argv[2] : NULL;
        if (!blog_name) SL_terminate(-1, "Expected blog name!");
    
        md_tree md = parse_md(strf(ROOT"generate/blogs/%s.md", blog_name));
        printf("\n------\n\n");
        // aforeach(e, md.elements) print_elem(e, 0);
        // printf("\n------\n\n");
        generate_blog(strf(ROOT"blogs/%s.html", blog_name), md);
        SBS_logI("is_french=%d, title=\"%s\"", md.is_french, md.title);
        
        printf("\n------\n\n");
        update_list(ROOT"blog.list", list_entry_(blog_name, time(NULL), md.is_french, md.title, md.description));
    }
    else 
    {
        SL_terminate(-1, "Expected generation command (\"blog\" or \"project\")!");
    }

    return 0;
}