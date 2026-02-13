#!/bin/bash

[ -z $1 ] && echo "Usage: $0 file-list" && exit

for file in $*
do
    cat $file | sed -rn \
        -e "s/^ \* (@fn )(.*)/\`\2\`/p" \
        -e "s/^ \* (@var )(.*)/\`\2\`/p" \
        -e "s/^ \* (@struct )(.*)/\`\2\`/p" \
        -e "s/^ \* (@brief )(.*)/\2/p" \
        -e "s/^ \* (@param )([a-z0-9_.]*)(.*)/* \`\2\` - \3/p" \
        -e "s/^ \* (@return )(.*)/\*\*Returns:\*\* \2/p" \
        -e "s/\s*(.*)[;|,]\s*\/\*\*< (.*) \*\//* \`\1\` - \2/p" \
        -e "s/^\/\*\*/---/p" \
        -e "s/^ \* (.*)/\1/p" \
        -e "s/^ \*$//p" \
        -e "s/^ \*\/$//p"
done
