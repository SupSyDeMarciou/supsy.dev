#include "../../SupSyBuildSystem/sbs.h"
#define SL_STRIP_PREFIX
#define SL_IMPLEMENTATION
#include <SL/sl.h>


typedef struct file_loc
{
    const char *source_path;
    usize p, l, c;
} file_loc;

#define info(loc, fmt, ...)  (fprintf(stdout, "%s:%02zu:%02zu - [INFO]  "fmt"\n", (loc).source_path, (loc).l + 0, (loc).c + 1, ##__VA_ARGS__))
#define error(loc, fmt, ...) (fprintf(stderr, "%s:%02zu:%02zu - [ERROR] "fmt"\n", (loc).source_path, (loc).l + 0, (loc).c + 1, ##__VA_ARGS__), exit(1))



typedef enum g_text_type
{
    TEXT_PLAIN  = 0b0000, // regular written text
    TEXT_BOLD   = 0b0001, // single stars
    TEXT_ITALIC = 0b0010, // double stars
    TEXT_CODE   = 0b0100, // monospace font with background
    TEXT_NEWL   = 0b1000, // newline
} g_text_type;

typedef enum g_elem_type
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
} g_elem_type;

typedef struct g_text  g_text;
typedef struct g_list  g_list;
typedef struct g_title g_title;
typedef struct g_para  g_para;
typedef struct g_code  g_code;
typedef struct g_box   g_box;
typedef struct g_elem  g_elem;

DEF_ARRAY(g_text);
DEF_ARRAY(g_list);
DEF_ARRAY(g_elem);


struct g_text
{
    file_loc loc;
    g_text_type type;
    char *str;
};

struct g_title
{
    usize depth;
    array(g_text) text;
};

struct g_para
{
    array(g_text) text;
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
    g_elem_type type;
    union {
        g_text  te;
        g_title ti;
        g_para  pa;
        g_list  li;
        g_code  co;
        g_box   bx;
    };
};
#define g_elem_(type_, loc_, ...) ((g_elem){.type = type_, .loc = loc_, ##__VA_ARGS__})

void print_elem(g_elem *elem, usize indent)
{
    switch (elem->type) {
        case ELEM_BREAK: info(elem->loc, "%*s<BREAK>", (int)indent * 4, ""); break;
        case ELEM_TEXT: {
            const char *tag = NULL;
            if (elem->te.type & TEXT_BOLD)   tag = tag ? strf("%s|bold", tag)   : "bold";
            if (elem->te.type & TEXT_ITALIC) tag = tag ? strf("%s|italic", tag) : "italic";
            if (elem->te.type & TEXT_CODE)   tag = tag ? strf("%s|code", tag)   : "code";
            
            if      (elem->te.type == 0)         info(elem->te.loc, "%*s\"%s\"", (int)indent * 4, "", elem->te.str);
            else if (elem->te.type == TEXT_NEWL) info(elem->te.loc, "%*s<br>", (int)indent * 4, "");
            else                                 info(elem->te.loc, "%*s<%s> \"%s\" </%s>", (int)indent * 4, "", tag, elem->te.str, tag);
        } break;
        case ELEM_TITLE: {
            char tag[32] = "################################";
            info(elem->loc, "%*s%.*s {", (int)indent * 4, "", (int)elem->ti.depth, tag);
            aforeach(te, elem->ti.text)
            {
                g_elem as_el = { .type = ELEM_TEXT, .te = *te }; 
                print_elem(&as_el, indent + 1);
            }
            info(elem->loc, "%*s}", (int)indent * 4, "");
        } break;
        case ELEM_PARAGRAPH: {
            info(elem->loc, "%*sp {", (int)indent * 4, "");
            aforeach(te, elem->pa.text)
            {
                g_elem as_el = { .type = ELEM_TEXT, .loc = te->loc, .te = *te }; 
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

    array(g_elem) elements;

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
    info(md->loc, "indent = %d, line = \"%.15s\" {...}", cur_indent, md->line);
    return md->line;
}

g_text parse_md_text(md_reader *md, g_text_type *type)
{   
    g_text text = { .loc = md->loc, .type = *type, .str = NULL };
    if (strcmp(&md_at(md), "  ") == 0)
    {
        text.type = TEXT_NEWL;
        md_mv(md, 2);
        return text;
    }

    const bool is_code   = (*type & TEXT_CODE)   == TEXT_CODE;
    const bool is_italic = (*type & TEXT_ITALIC) == TEXT_ITALIC;
    const bool is_bold   = (*type & TEXT_BOLD)   == TEXT_BOLD;
    
    // info(md->loc, "parse_md_text(is_bold=%d, is_italic=%d, is_code=%d) = \"%s\"", is_bold, is_italic, is_code, md->line + md->loc.c);
    while (md_at(md))
    {
        if (!is_code && !is_italic && md_at(md) == '*' && md_at(md, 1) == '*')
        {
            if (is_bold)
            {
                if (text.loc.c != md->loc.c) text.str = strndup(md->line + text.loc.c, md->loc.c - text.loc.c);
                md_mv(md, 2);
                *type &= ~TEXT_BOLD;
                return text;
            }
            if (!isspace(md_at(md, 2))) {
                md_mv(md, 2);
                *type |= TEXT_BOLD;
                if (text.loc.c == md->loc.c - 2) return parse_md_text(md, type);

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
                *type &= ~TEXT_ITALIC;
                return text;
            }
            if (!isspace(md_at(md, 1))) {
                md_mv(md, 1);
                *type |= TEXT_ITALIC;
                if (text.loc.c == md->loc.c - 1) return parse_md_text(md, type);

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
                *type &= ~TEXT_CODE;
                return text;
            }
            if (!isspace(md_at(md, 1))) {
                md_mv(md, 1);
                *type |= TEXT_CODE;
                if (text.loc.c == md->loc.c - 1) return parse_md_text(md, type);

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
    usize ordered_label = 0;
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
            g_text_type t_type = 0;
            while (md_at(md))
            {
                g_text t = parse_md_text(md, &t_type);
                if (t.str == NULL) continue;
                arrayAdd(title.ti.text, t);
                if (t.type == TEXT_NEWL) break;
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
            do ++ordered_label; while (isdigit(md_at(md, ordered_label)));
            if (md_at(md, ordered_label) != '.' || !isspace(md_at(md, ordered_label + 1))) goto TEXT;
            goto LIST;
        } 
        case '-':
        {
            if (!isspace(md_at(md, 1))) goto TEXT;
        LIST:
            if (md->list_stack.count == 0 || arrayLast(md->list_stack)->indent < cindent)
            {
                printf("[LIST] Creating new list\n");

                file_loc list_loc = md->loc;
                g_list *list = arrayAdd(md->list_stack, ((g_list) {
                    .indent = cindent, .is_ordered = ordered_label > 0
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
                    printf("[LIST] Propagate \"break\"\n");
                    file_loc loc = md->loc;
                    loc.c = depth;
                    arrayAdd(*elements, g_elem_(ELEM_BREAK, loc));
                }
                
                return true;
            }

            printf("[LIST] Adding new item\n");
            
            arrayAdd(*elements, g_elem_(ELEM_LIST_ITEM, md->loc, .li.is_ordered = ordered_label > 0));
            
            md_mv(md, ordered_label);
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

            g_text_type t_type = 0;
            while (md_at(md))
            {
                g_text t = parse_md_text(md, &t_type);
                if (t.type == TEXT_NEWL) arrayAdd(paragraph.pa.text, t);
                if (t.str == NULL) continue;
                arrayAdd(paragraph.pa.text, t);
            }

            if (concat) arrayLast(*elements)->pa = paragraph.pa;
            else arrayAdd(*elements, paragraph);
        } break;
    }

    return true;
}

array(g_elem) parse_md(const char *source_path)
{
    FILE *f = fopen(source_path, "r");
    if (!f) SL_terminate(-1, "Failed to open \"%s\"!", source_path);

    md_reader md = { .f = f, .loc = { .source_path = source_path } };

    while (
        readline(&md) &&
        parse_md_elem(&md, &md.elements, md.indent, -1, 0)
    );

    fclose(f);    
    return md.elements;
}



bool generate_html_text(FILE *f, g_text text)
{
    if (text.type == TEXT_NEWL) 
    {
        fprintf(f, "<br>");
        return true;
    }
    
    if (text.type & TEXT_BOLD)   fputs("<b>", f);
    if (text.type & TEXT_ITALIC) fputs("<i>", f);
    if (text.type & TEXT_CODE)   fputs("<code>", f);
    for (char *p = text.str; *p; ++p)
    {
        if      (*p == '&') fputs("&amp", f);
        else if (*p == '<') fputs("&lt", f);
        else                fputc(*p, f);
    }
    if (text.type & TEXT_CODE)   fputs("</code>", f);
    if (text.type & TEXT_ITALIC) fputs("</i>", f);
    if (text.type & TEXT_BOLD)   fputs("</b>", f);

    return false;
}

void generate_html_elem(FILE *f, usize depth, g_elem elem, bool first)
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

                generate_html_elem(f, depth + 2, *el, false);
            }
            fprintf(f, "%*s</li>\n", ((int)depth + 1) * 4, "");
            fprintf(f, "%*s</%cl>\n", (int)depth * 4, "", uo);
        } break;

        case ELEM_PARAGRAPH:
        {
            fprintf(f, "%*s<p>\n%*s", (int)depth * 4, "", ((int)depth + 1) * 4, "");
            aforeach(text, elem.pa.text)
            {
                if (generate_html_text(f, *text))
                    fprintf(f, "\n%*s", ((int)depth + 1) * 4, "");
            }
            fprintf(f, "\n%*s</p>\n", (int)depth * 4, "");
        } break;

        case ELEM_TITLE: 
        {
            if (!first && elem.ti.depth == 1) {
                fprintf(f, "%*s</section>", ((int)depth - 1) * 4, "");
                fprintf(f, "%*s<section class=\"section-base blog\">", ((int)depth - 1) * 4, "");
            }

            fprintf(f, "%*s<h%zu>", (int)depth * 4, "", elem.ti.depth);
            aforeach(text, elem.ti.text) generate_html_text(f, *text);
            fprintf(f, "</h%zu>\n", elem.ti.depth);
        } break;

        case ELEM_CODE:
        {
            fprintf(f, "%*s<pre class=\"code-box\">\n", (int)depth * 4, "");
            aforeach(line, elem.co.lines) {
                for (char *p = *line; *p; ++p)
                {
                    if      (*p == '&') fputs("&amp", f);
                    else if (*p == '<') fputs("&lt", f);
                    else                fputc(*p, f);
                }
                fputc('\n', f);
            }
            fprintf(f, "%*s</pre>\n", (int)depth * 4, "");
        } break;

        case ELEM_BOX:
        {
            fprintf(f, "%*s<hgroup>\n", (int)depth * 4, "");
            // fprintf(f, "%*s<div class=\"box\">\n", (int)depth * 4, "");
            aforeach(el, elem.bx.items) generate_html_elem(f, depth + 1, *el, false);
            fprintf(f, "%*s</hgroup>\n", (int)depth * 4, "");
            // fprintf(f, "%*s</div>\n", (int)depth * 4, "");
        } break;

        default: break;
    }
}

void generate_html(const char *output_path, array(g_elem) elements)
{
    // FILE *f = stdout;
    FILE *f = fopen(output_path, "w");
    if (!f) SL_terminate(-1, "Failed to open \"%s\"!", output_path);
    
    fprintf(f, 
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <link rel=\"stylesheet\" href=\"../style.css\" type=\"text/css\">\n"
        "    <!-- <link rel=\"icon\" href=\"icons/supsy.png\"> -->\n"
        "    <title>supsy.dev</title>\n"
        "    <!-- <FONT> -->\n"
        "        <link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\n"
        "        <link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>\n"
        "        <link href=\"https://fonts.googleapis.com/css2?family=Atkinson+Hyperlegible+Mono:ital,wght@0,293;1,293&display=swap\" rel=\"stylesheet\">\n"
        "    <!-- </FONT> -->\n"
        "</head>\n"
        "<body>\n"
        "    <main>\n"
        "        <header class=\"section-base\">\n"
        "            <p><a href=\"../index.html\">SupSy<span class=\"text-accent\">.dev</span></a></p>\n"
        "            <p class=\"no-select\">|</p>\n"
        "            <p><a href=\"../blog.html\">Blog</a></p>\n"
        "            <p class=\"no-select\">|</p>\n"
        "            <p><a href=\"../projects.html\">Projects</a></p>\n"
        "        </header>\n"
        "        <section class=\"section-base blog\">\n"

    );
    aforeach(el, elements) generate_html_elem(f, 3, *el, aindex(el) == 0);
    fprintf(f,
        "        </section>\n"
        "        <footer><div></div><p>Vlad MUNTEANU 2026 - All rights reserved.</p></footer>\n"
        "    </main>\n"
        "</body>\n"
        "</html>"
    );

    fclose(f);
}

#define ROOT "/home/supsy/Coding/SupSy/supsy.dev/"

int main(int argc, char **argv)
{
    SBS_rebuild(argc, argv, "-I../../SupSyLibraries/include/");

    const char *blog_name = argc > 1 ? argv[1] : NULL;
    if (!blog_name) SL_terminate(-1, "Expected blog name!");

    array(g_elem) elements = parse_md(tmpf(ROOT"generate/blogs/%s.md", blog_name));
    // printf("\n------\n\n");
    // aforeach(e, elements) print_elem(e, 0);
    generate_html(tmpf(ROOT"blogs/%s.html", blog_name), elements);

    return 0;
}