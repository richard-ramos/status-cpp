from conans import ConanFile

class ConanPackage(ConanFile):
    name = 'status-cpp'
    version = "0.1.0"

    generators = 'cmake_find_package'

    requires = [
        ('openssl/1.1.1h'),
        ('boost/1.75.0')
    ]

    default_options = (
        'boost:shared=False',
    )
