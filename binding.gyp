{
    "targets": [
        {
            "target_name": "gpgme",
            "sources": [ "src/addon.cc",
                         "src/context.cc",
                         ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")",
                "<!@(gpgme-config --cflags | sed 's/-I//g')"
            ],
            "link_settings": {
                "libraries": [
                    "<!@(gpgme-config --libs | sed 's/-L[A-Za-z0-9\.\/\-\_]*[:blank:]*//g')"
                ],
                "library_dirs": [
                   "<!@(gpgme-config --libs | sed 's/-l[a-z\-]*[:blank:]*//g' | sed 's/-L//g')"

                ]    
 
            }
        }
    ]
}