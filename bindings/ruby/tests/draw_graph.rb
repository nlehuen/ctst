require 'rubygems'
require 'rtst'

words = %w{
colored
colours
comitee
community
commuter
commuters
compacity
compact
compacted
compute
continuity
}

r = RTST.new

words.each do |word|
    r.set(word, true)
end

r.dump 'graph.dot'