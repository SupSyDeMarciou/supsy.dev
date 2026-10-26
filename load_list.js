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

function insertListEntries(filePath, into, max=-1, overflow_redirect_link="") 
{
    const blog_list_lines = loadFile(filePath).split("\n");
    var nb_entries = 0;
    for (var i = 0; i + 5 < blog_list_lines.length;)
    {
        while (i < blog_list_lines.length && blog_list_lines[i].trimStart().length == 0) ++i;
        if (i >= blog_list_lines.length) break;
    
        const entry = {
            name:  blog_list_lines[i++],
            date:  blog_list_lines[i++],
            lang:  blog_list_lines[i++],
            title: blog_list_lines[i++],
            desc:  blog_list_lines[i++]
        }
    
        console.log(entry)
    
        const list_entry = document.createElement("li");
        list_entry.innerHTML =
            "<h4>" +
                "[" + entry.date + " - " + entry.lang.toUpperCase() + "] " + 
                "<a class=\"text-accent underline\" href=\"blogs/" + entry.name + ".html\">" + entry.title + "</a> :" +
            "</h4>" +
            entry.desc

        into.appendChild(list_entry)
        
        if (max > 0 && ++nb_entries >= max) {
            const overflow_entry = document.createElement("a");
            overflow_entry.classList.add("text-dim");
            overflow_entry.setAttribute("href", overflow_redirect_link)
            overflow_entry.innerText = "And more..."
            into.appendChild(overflow_entry)
            break;
        }
    }
}