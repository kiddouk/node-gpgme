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
                ],
                "library_dirs": [
                ]    
 
            }
        }
    ]
}