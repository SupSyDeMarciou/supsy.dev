function loadFile(filePath) 
{
    // https://stackoverflow.com/a/41133213
    var result = null;
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET", filePath, false);
    xmlhttp.send();
    if (xmlhttp.status==200) result = xmlhttp.responseText;
    return result;
}

function insertListEntries(category, into, max=-1) 
{
    const blog_list_lines = loadFile(category + ".list").split("\n");
    var nb_entries = 0;
    for (var i = 0; i + 6 < blog_list_lines.length;)
    {
        while (i < blog_list_lines.length && blog_list_lines[i].trimStart().length == 0) ++i;
        if (i >= blog_list_lines.length) break;
    
        const entry = {
            name:          blog_list_lines[i++],
            date_created:  blog_list_lines[i++],
            date_modified: blog_list_lines[i++],
            lang:          blog_list_lines[i++],
            title:         blog_list_lines[i++],
            description:   blog_list_lines[i++]
        }
    
        console.log(entry)
    
        const list_entry = document.createElement("li");
        list_entry.innerHTML =
            "<h4>" +
                "[" + entry.date_created + " - " + entry.lang.toUpperCase() + "] " + 
                "<a class=\"text-accent underline\" href=\""+category+"s/" + entry.name + "/index.html\">" + entry.title + "</a> :" +
            "</h4>" +
            entry.description + 
            (entry.date_modified.startsWith(entry.date_created) ? "" : "<br/><i class=\"text-dim\"> Last modified: " + entry.date_modified + "</i>")

        into.appendChild(list_entry)
        
        if (max > 0 && ++nb_entries >= max) {
            const overflow_entry = document.createElement("a");
            overflow_entry.classList.add("text-dim");
            overflow_entry.setAttribute("href", category.startsWith("project") ? "projects.html" : "blog.html")
            overflow_entry.innerText = "And more..."
            into.appendChild(overflow_entry)
            break;
        }
    }
}