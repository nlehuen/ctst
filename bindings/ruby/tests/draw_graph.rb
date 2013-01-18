require 'rubygems'
require 'rtst'

words = %w{
colours
colored
comitee
comitees
compacity
compact
compacted
community
commuter
commuters
compute
}

r = RTST.new

words.each do |word|
    r.set(word, true)
end

r.dump 'graph.dot'