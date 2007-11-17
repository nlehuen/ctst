require 'rtst'

v = []
t = RTST.new

10000.times do
  r = rand.to_s
  v << r
  t.set(r,r)
end

removed = []
2000.times do
  r = rand(v.size)
  to_remove = v.delete_at(r)
  removed << to_remove
  t.remove(to_remove)
end

v.each do |r|
  fail unless t.get(r) == r
end

removed.each do |r|
  result = t.get(r)
  fail "Not removed : #{result}" unless result == nil
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

