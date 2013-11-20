import qbs.base 1.0

Application {
    name: 'NXMHandler'

    Depends { name: 'Qt.core' }
    Depends { name: 'Qt.gui' }
    Depends { name: 'UIBase' }
    Depends { name: 'cpp' }

    cpp.defines: []
    cpp.includePaths: [ '../uibase', qbs.getenv("BOOSTPATH") ]

    cpp.staticLibraries: [ 'shell32' ]

    Group {
        name: 'Headers'
        files: [ '*.h' ]
    }

    Group {
        name: 'Sources'
        files: [ '*.cpp' ]
    }

    Group {
        name: 'UI Files'
        files: [ '*.ui' ]
    }
}