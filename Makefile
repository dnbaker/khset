CXXFLAGS += -std=c++17 -DKH_USE_64_BIT

khtest: khtest.cpp khset.h
	$(CXX) $(CXXFLAGS) -I. $< -o $@ -lz && ./khtest
khtest_d: khtest.cpp khset.h
	$(CXX) $(CXXFLAGS) -fsanitize=address -fsanitize=undefined -I. $< -o $@
