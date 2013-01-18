# Loads mkmf which is used to make makefiles for Ruby extensions
require 'mkmf'

# Give it a name
extension_name = 'rtst'

# The destination
dir_config(extension_name,".")

# Do the work
with_cflags('-Wall -O3') do
    create_makefile(extension_name)
end