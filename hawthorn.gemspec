Gem::Specification.new do |s|
	s.name = 		'hawthorn'
	s.version = 	'0.0.1'
	s.date = 		'2012-09-26'
	s.summary = 	'Hawthorn'
	s.description = 'Hawthorn Graph Database'
	s.authors = 	['Markus Gronholm']
	s.email = 		'markus@alshain.fi'
	s.files = Dir.glob('lib/**/*.rb') + Dir.glob('ext/**/*.{c,h,rb}')
    s.extensions = ['ext/hawthorn/extconf.rb']
end

