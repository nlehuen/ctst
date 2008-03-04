require 'rtst'

v = []
t = RTST.new

100000.times do
  r = rand.to_s
  v << r
  t.set(r,r)
end

v.each do |r|
  result = t.get(r)
  fail "Problem with key : '#{r}'!='#{result}'" unless r == result
end

removed = {}
20000.times do
  r = rand(v.size)
  to_remove = v.delete_at(r)
  removed[to_remove] = true
  t.remove(to_remove)
end

removed.each do |r,dummy|
  result = t.get(r)
  fail "Not removed : '#{result}'" unless result == nil
end

v.each do |r|
  result = t.get(r)
  fail "Problem with key : '#{r}'!='#{result}'" unless r == result
end

result = t.each do |key, value, distance|
  fail unless key == value
  fail unless distance == 0
end

fail result.inspect unless result == nil

puts "ALL GOOD !"
puts "Number of keys : #{t.size}"
puts "Total key length : #{t.total_key_length}"
puts "Number of nodes : #{t.node_count}"
puts "Memory usage : #{t.memory_usage}"
