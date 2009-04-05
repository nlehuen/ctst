require 'rtst'

$counter = 0
def count()
   puts "COUNT: #{$counter}"
   $counter += 1
end

count

v = []
t = RTST.new

count

100000.times do
  r = rand.to_s
  v << r
  t.set(r,r)
end

count

v.each do |r|
  result = t.get(r)
  puts "Cool !" if r == result
  fail "Problem with key : '#{r}'!='#{result}'" unless r == result
end

count

removed = {}
20000.times do
  r = rand(v.size)
  to_remove = v.delete_at(r)
  removed[to_remove] = true
  t.remove(to_remove)
end

count

removed.each do |r,dummy|
  result = t.get(r)
  fail "Not removed : '#{result}'" unless result == nil
end

count

v.each do |r|
  result = t.get(r)
  fail "Problem with key : '#{r}'!='#{result}'" unless r == result
end

count

result = t.each do |key, value, distance|
  fail unless key == value
  fail unless distance == 0
end

count

fail result.inspect unless result == nil

count

puts "ALL GOOD !"
puts "Number of keys : #{t.size}"
puts "Total key length : #{t.total_key_length}"
puts "Number of nodes : #{t.node_count}"
puts "Memory usage : #{t.memory_usage}"

