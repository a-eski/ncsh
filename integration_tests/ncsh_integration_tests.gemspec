# frozen_string_literal: true

Gem::Specification.new do |spec|
  spec.name          = 'ncsh_integration_tests'
  spec.version       = 1
  spec.authors       = ['Alex Eski']
  spec.email         = ['alexeski@gmail.com']

  spec.summary       = 'Integration tests for the ncsh project.'
  spec.description   = 'Uses ttytest2 to test the ncsh project.'
  spec.homepage      = 'https://github.com/a-eski/ncsh'
  spec.license       = 'GPLv3.0, MIT'

  spec.files         = `git ls-files -z`.split("\x0").reject do |f|
    f.match(%r{^(test|spec|features)/})
  end
  spec.bindir        = 'exe'
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ['./']

  spec.required_ruby_version = '>= 3.2.3'

  spec.add_development_dependency 'bundler', '~> 2.5'
  spec.add_development_dependency 'minitest', '~> 5.0'
  spec.add_development_dependency 'rake', '~> 13.0'
end
