rm *.dot *.png
make
ruby test_rtst.rb
find -name "*.dot" -exec dot -Tpng -O {} \; -delete
rm *.dot
