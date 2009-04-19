require 'rtst'

srand(43776)

v = []
t = RTST.new

count = 0
1000.times do
  count = count + 1
  
  r = rand.to_s

  v << r
  t.set(r,r)
  
  t.dump "tree.#{count}.dot"
  
  fail "Not stored : #{r}" if t.get(r) != r

  v.each do |r|
    result = t.get(r)
    fail "Problem with key : '#{r}'!='#{result}'" unless r == result
  end
end

v.each do |r|
  result = t.get(r)
  fail "Problem with key : '#{r}'!='#{result}'" unless r == result
end

removed = {}
200.times do
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

