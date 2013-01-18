# rtst

`rtst` is a gem providing Ruby bindings to `ctst`.

## Installation

Add this line to your application's Gemfile:

    gem 'rtst'

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install rtst

## Usage

```ruby
require 'rubygems'
require 'rtst'

t = RTST.new

# Set values
t.set 'key1', 'Hello'
t.set 'key2', 'World'

# Get values
puts "#{t.get('key1')} #{t.get('key2')}"

# Iterate on keys (in alphabetical order)
t.each do |key, value, distance|
    puts "#{key}: #{value}"
end
```

## Contributing

1. Fork it
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create new Pull Request
