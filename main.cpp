#include <cstdint>
#include <array>
#include <iostream>
#include <chrono>

constexpr bool test(std::uint32_t value, std::uint32_t mask)noexcept{
	return (value & (1 << mask)) != 0;
}

constexpr std::uint32_t bitcnt(std::uint32_t value){
	value = (value & 0x5555) + ((value >> 1) & 0x5555);
	value = (value & 0x3333) + ((value >> 2) & 0x3333);
	value = (value & 0x0f0f) + ((value >> 4) & 0x0f0f);
	return (value & 0x00ff) + ((value >> 8) & 0x00ff);
}

constexpr std::uint32_t msb(std::uint32_t value){
	value |= (value >> 1);
	value |= (value >> 2);
	value |= (value >> 4);
	value |= (value >> 8);
	return -1 + bitcnt(value);
}

using Stage = std::uint32_t[9][9];

//Stage stage{
//	{0,6,1,0,9,5,0,0,0},
//	{0,0,0,0,1,4,0,9,7},
//	{4,0,0,0,0,0,0,0,5},
//	{9,0,0,0,2,0,0,5,0},
//	{0,0,0,0,0,0,0,0,0},
//	{0,3,0,0,6,0,0,0,2},
//	{1,0,0,0,0,0,0,0,9},
//	{2,8,0,7,3,0,0,0,0},
//	{0,0,0,2,5,0,6,7,0},
//};
Stage stage{
	{0, 0, 0, 8, 0, 0, 0, 3, 1},
	{0, 0, 0, 3, 6, 0, 2, 0, 0},
	{0, 0, 7, 0, 0, 0, 0, 0, 0},
	{0, 0, 4, 0, 0, 1, 5, 7, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 6, 3, 7, 0, 0, 4, 0, 0},
	{0, 0, 0, 0, 0, 0, 7, 0, 0},
	{0, 0, 6, 0, 8, 9, 0, 0, 0},
	{9, 8, 0, 0, 0, 2, 0, 0, 0},
};


class App{
	Stage value;
	Stage expect;
	int fill_count = 0;
public:
	void init(){
		for(std::uint32_t iy = 0; iy < 9; ++iy){
			for(std::uint32_t ix = 0; ix < 9; ++ix){
				value[iy][ix] = 0;
				expect[iy][ix] = 0b1111111110;
			}
		}
		for(std::uint32_t iy = 0; iy < 9; ++iy){
			for(std::uint32_t ix = 0; ix < 9; ++ix){
				if(stage[iy][ix] != 0){
					set(stage[iy][ix], ix, iy);
				}
			}
		}
	}
	bool check(){
		for(std::uint32_t iy = 0; iy < 9; ++iy){
			for(std::uint32_t ix = 0; ix < 9; ++ix){
				if((value[iy][ix] == 0) == (expect[iy][ix] == 0)){
					return false;
				}
			}
		}
		return true;
	}
	bool run()noexcept{
		bool is_advance = true;
		while(fill_count < 9 * 9){
			if(!check()){
				return false;
			}
			if(!is_advance){
				App app = *this;
				for(std::uint32_t iy = 0; iy < 9; ++iy){
					if(is_advance){
						break;
					}
					for(std::uint32_t ix = 0; ix < 9; ++ix){
						if(is_advance){
							break;
						}
						if(app.value[iy][ix] == 0){
							if(app.expect[iy][ix] == 0){
								return false;
							}
							auto const v = msb(app.expect[iy][ix]);
							app.set(v, ix, iy);
							if(app.run()){
								*this = app;
								return true;
							}
							else{
								expect[iy][ix] ^= (1 << v);
								is_advance = true;
								continue;
							}
						}
					}
				}
			}
			is_advance = false;
			for(std::uint32_t iy = 0; iy < 9; ++iy){
				for(std::uint32_t ix = 0; ix < 9; ++ix){
					if(value[iy][ix] == 0){
						if(bitcnt(expect[iy][ix]) == 1){
							set(msb(expect[iy][ix]), ix, iy);
							is_advance = true;
						}
					}
				}
			}
			for(std::uint32_t iy = 0; iy < 9; ++iy){
				for(std::uint32_t ix = 0; ix < 8; ++ix){
					std::uint32_t count = 1;
					bool eq[9] = {};
					std::uint32_t const v = expect[iy][ix];
					eq[ix] = true;
					for(std::uint32_t jx = ix + 1; jx < 9; ++jx){
						if(v == expect[iy][jx]){
							eq[jx] = true;
							count++;
						}
					}
					if(bitcnt(v) == count){
						for(std::uint32_t jx = 0; jx < 9; ++jx){
							if(!eq[jx]){
								expect[iy][jx] &= ~v;
							}
						}
					}
				}
			}
			for(std::uint32_t ix = 0; ix < 9; ++ix){
				for(std::uint32_t iy = 0; iy < 8; ++iy){
					std::uint32_t count = 1;
					bool eq[9] = {};
					std::uint32_t const v = expect[iy][ix];
					eq[iy] = true;
					for(std::uint32_t jy = iy + 1; jy < 9; ++jy){
						if(v == expect[jy][ix]){
							eq[jy] = true;
							count++;
						}
					}
					if(bitcnt(v) == count){
						for(std::uint32_t jy = 0; jy < 9; ++jy){
							if(!eq[jy]){
								expect[jy][ix] &= ~v;
							}
						}
					}
				}
			}
			for(std::uint32_t iy = 0; iy < 9; ++iy){
				for(std::uint32_t ix = 0; ix < 9; ++ix){
					std::uint32_t count = 0;
					bool eq[9] = {};
					std::uint32_t const v = expect[iy][ix];
					eq[ix] = true;
					for(std::uint32_t jx = 0; jx < 9; ++jx){
						auto const y = iy / 3 * 3 + jx / 3;
						auto const x = ix / 3 * 3 + jx % 3;
						if(v == expect[y][x]){
							eq[jx] = true;
							count++;
						}
					}
					if(bitcnt(v) == count){
						if(v == 12){
							int a = 0;
						}
						for(std::uint32_t jx = 0; jx < 9; ++jx){
							if(!eq[jx]){
								auto const y = iy / 3 * 3 + jx / 3;
								auto const x = ix / 3 * 3 + jx % 3;
								expect[y][x] &= ~v;
							}
						}
					}
				}
			}
		}
		return true;
	}
	void set(std::uint32_t a, std::uint32_t x, std::uint32_t y)noexcept{
		++fill_count;
		value[y][x] = a;
		expect[y][x] = 0;
		auto ex = [&](auto& dest){
			dest &= dest ^ (1 << a);
		};
		for(auto& t : expect[y]){
			ex(t);
		}
		for(std::uint32_t i = 0; i < 9; ++i){
			ex(expect[i][x]);
		}
		std::uint32_t const block_x = x / 3 * 3;
		std::uint32_t const block_y = y / 3 * 3;
		for(std::uint32_t iy = 0; iy < 3; ++iy){
			for(std::uint32_t ix = 0; ix < 3; ++ix){
				ex(expect[block_y + iy][block_x + ix]);
			}
		}
	}
	void print(){
		for(std::uint32_t iy = 0; iy < 9; ++iy){
			for(std::uint32_t ix = 0; ix < 9; ++ix){
				std::cout << value[iy][ix] << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}
};


int main(){
	std::chrono::system_clock::time_point start, end;
	start = std::chrono::system_clock::now();
	App app;
	app.init();	
	app.run();
	end = std::chrono::system_clock::now();
	app.print();
	std::cout << static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) << "microseconds" << std::endl;
}
