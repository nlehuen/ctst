# -*- encoding: utf-8 -*-
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'rtst/version'

# Copy source files
require 'fileutils'
FileUtils.cp_r '../../src/.', 'ext/rtst'

Gem::Specification.new do |gem|
  gem.name          = "rtst"
  gem.version       = Rtst::VERSION
  gem.authors       = ["Nicolas Lehuen"]
  gem.email         = ["nicolas@lehuen.com"]
  gem.description   = %q{ctst bindings for Ruby}
  gem.summary       = %q{ctst bindings for Ruby}
  gem.homepage      = ""

  gem.files         = Dir.glob('ext/**/*.{c,h,rb}')
  gem.extensions    = ['ext/rtst/extconf.rb']
  gem.executables   = gem.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  gem.test_files    = gem.files.grep(%r{^(test|spec|features)/})
  gem.require_paths = ["lib"]
end


