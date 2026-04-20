#!/bin/bash

if [ -z $1 ]
then
    echo "Doxygen to markdown: $0 -d c-file-list"
    echo "Markdown table of contents: $0 -c md-file-list"
    echo "Markdown to HTML: $0 -x file.md"
    exit
fi

if [ $1 == "-d" ]
then
    shift
    for file in $*
    do
        sed -En '
            s|^ \* (@fn )(.*)|`\2`|p
            s|^ \* (@var )(.*)|`\2`|p
            s|^ \* (@type )(.*)|`\2`|p
            s|^ \* (@brief )(.*)|\2|p
            s|^ \* (@param )([a-z0-9_.]*)(.*)|* `\2` - \3|p
            s|^ \* (@return )(.*)|**Returns:** \2|p
            s|^\s*(.*)[;|,]\s*\/\*\*< (.*) \*\/|* `\1` - \2|p
            s|^\/\*\*|---|p
            s|^ \* (.*)|\1|p
            s|^ \*$||p
            s|^ \*\/$||p' $file
    done |
        # fixup field description lists
        sed -E '/^\* /,/^$|^[^\*]/{/^[^\*]/s|(.*)|\n\1|}'

elif [ $1 == "-c" ]
then
    shift
    for file in $*
    do
        sed -En "s|(^# )(.*)|\#\#\# [\2]($file)|p" $file
        sed -En "s|(^## )(.*)|\2|p" $file | while read -r line
        do
            link=$(echo $line | tr "[:upper:] " "[:lower:]-")
            echo "* [$line]($file\#$link)"
        done
    done

elif [ $1 == "-x" ]
then
    [ -z $2 ] && exit
    cat $2 |
    # phase 1: headers and line break
    sed -E '
        s|^---|<hr>|
        s|^# (.*)|<h1>\1<\/h1>|
        s|^## (.*)|<h2>\1<\/h2>|
        s|^### (.*)|<h3>\1<\/h3>|' |\
    # phase 2: code paragraphs
    sed -E '
        /^```/{:1 N; s|```(.*)```|\n<p><code>\1<\/code><\/p>\n|; T1}' |\
    # phase 3: paragraphs and lists
    sed -E '/./{H;$!d} ; x
        s|^\n```(.*)```|<p><code>\1<\/code><\/p>|
        s|^(\n\* .*)|<ul>\1\n<\/ul>|
        s|^(\n[^<].*)|<p>\1\n<\/p>|
        s|^\n(.*)|\1|' |\
    # phase 4: bullets and highlighting
    sed -E '
        s|^\* (.*)$|<li>\1<\/li>|
        s|`([^`]*)`|<code>\1<\/code>|g
        s|\*\*([^\*]*)\*\*(\s\|$)|<strong>\1<\/strong>\2|g
        s|_([^_]*)_(\s\|$)|<em>\1<\/em>\2|g
        s|\[(.*)\]\((.*)\)|<a href="\2">\1</a>|g
        s|  $|<br>|'
fi
