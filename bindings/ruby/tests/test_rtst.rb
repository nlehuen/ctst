require 'rtst'
require 'set'
require 'test/unit'

class RTSTTests < Test::Unit::TestCase
	def setup
	  # set this if you want reproducible runs
	  # srand 43776

      # 1000 unique, random strings
	  @items = Set.new	  
	  while @items.size < 1000
	  	item = rand.to_s
	  	@items << item
	  end
	  	  
	  @tree = RTST.new
	end
	
	def teardown
	end
	
	def test_fill
		@items.each do |item|
			assert_nil @tree.set(item, item)
		end
	end

	def test_read
		test_fill
		@items.each do |item|
			assert_equal item, @tree.get(item)
		end
	end
	
	def test_remove
		test_fill

		@removed = Set.new
		@items.first(200).each do |item|
			@removed << item
			@tree.remove item
		end
		
		@items.each do |item|
			if @removed.include? item
				assert_nil @tree.get(item)
			else
				assert_equal item, @tree.get(item)
			end
		end			
	end
	
	def test_each
		test_remove
		
		result = @tree.each do |key, value, distance|
			assert_equal key, value
			assert_equal 0, distance
			assert @items.include? key
			assert !@removed.include?(key)
    		@items.delete key
    		nil # Need to return nil to go through all the tree
		end
		
		assert_equal @removed.size, @items.size
	end
end
