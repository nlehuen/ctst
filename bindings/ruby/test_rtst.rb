require 'rtst'

v = []
t = RTST.new

1000000.times do
  r = rand.to_s
  v << r
  t.set(r,r)
end

removed = []
20000.times do
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

puts "ALL GOOD !"
puts "Number of keys : #{t.size}"
puts "Total key length : #{t.total_key_length}"
puts "Number of nodes : #{t.node_count}"
puts "Memory usage : #{t.memory_usage}"
