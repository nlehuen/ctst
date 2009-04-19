require 'rtst'
require 'set'
require 'fileutils'
require 'test/unit'

class RTSTTests < Test::Unit::TestCase
	def setup
	  # set this if you want reproducible runs
	  # srand 43776

      # 1000 unique, random strings
	  @items = Set.new	  
	  while @items.size < 1000
	  	item = rand.to_s + rand.to_s + rand.to_s + rand.to_s
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

	def dump_tree(tree,filename)
	    FileUtils.rm([filename,"#{filename}.png"], :force => true)
	    tree.dump(filename)
	    assert File.exists?(filename)
	    system("dot -Gcharset=latin1 -Kdot -Tpng -O #{filename}")
	    FileUtils.rm([filename])
	end

	def test_compacity
	    words = %w{
	    	compact
	    	compacity
	    	community
	    	commuter
	    	commuters
	    	compute
	    	compu
	    }
	    
	    words.each do |word|
	    	@tree.set word, 1
	    end
	    
	    dump_tree @tree, "test_compacity.dot"
	end

	def test_dump_and_graphviz
		@items.first(20).each do |item|
			@tree.set item, item
	    end
	    
	    dump_tree @tree, "test_rtst_dump.dot"
	end
	
	def test_french_dictionary
		@items = []
		
		words = IO.readlines("|zcat liste.de.mots.francais.frgut.txt.gz")
		words.shuffle!
			
		words.first(32).each do |line|
			line = line.strip
			assert_nil @tree.set(line,line)
		end

	    dump_tree @tree, "francais.dot"

		words.first(32).each do |line|
			line = line.strip
			assert_equal line, @tree.get(line)
		end

	end
end
