#!/bin/bash

cat helpers.h | grep "^ \*" | \
sed -r \
    -e "s/.*(@fn )(.*)/\n\`\2\`/" \
    -e "s/.*(@brief )(.*)/\2/" \
    -e "s/.*(@param )([a-z0-9_]*)(.*)/* \`\2\` - \3/" \
    -e "s/.*(@return )(.*)/\*\*Returns:\*\* \2/" \
    -e "s/^ \* (.*)/\1/" \
    -e "s/^ \*(.*)//" > README.md
