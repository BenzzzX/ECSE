#pragma once
#include "ECS\EEC.hpp"
#include<tuple>


namespace EEC::MPL
{

	template <class T, class... TArgs> decltype(void(T{ std::declval<TArgs>()... }), std::true_type{}) test_is_braces_constructible(int);
	template <class, class...> std::false_type test_is_braces_constructible(...);
	template <class T, class... TArgs> using is_braces_constructible = decltype(test_is_braces_constructible<T, TArgs...>(0));

	struct any_type {
		template<class T>
		constexpr operator T(); // non explicit
	};

	template<class T>
	auto to_tuple(T&& object) noexcept {
		using type = std::decay_t<T>;
		if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62, p63] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62, p63);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9, p10] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8, p9] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7, p8] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6, p7] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6, p7);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5, p6] = object;
			return std::make_tuple(p1, p2, p3, p4, p5, p6);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4, p5] = object;
			return std::make_tuple(p1, p2, p3, p4, p5);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3, p4] = object;
			return std::make_tuple(p1, p2, p3, p4);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type, any_type>{}) {
			auto&&[p1, p2, p3] = object;
			return std::make_tuple(p1, p2, p3);
		}
		else if constexpr(is_braces_constructible<type, any_type, any_type>{}) {
			auto&&[p1, p2] = object;
			return std::make_tuple(p1, p2);
		}
		else if constexpr(is_braces_constructible<type, any_type>{}) {
			auto&&[p1] = object;
			return std::make_tuple(p1);
		}
		else {
			return std::make_tuple();
		}
	}
	
	template<class S, std::size_t...Is, class Tup>
	S to_struct(std::index_sequence<Is...>, Tup&& tup) {
		using std::get;
		return { get<Is>(std::forward<Tup>(tup))... };
	}
	template<class S, class Tup>
	S to_struct(Tup&& tup) {
		using T = std::remove_reference_t<Tup>;

		return to_struct<S>(
			std::make_index_sequence<std::tuple_size<T>{}>{},
			std::forward<Tup>(tup)
		);
	}
	
	template<typename T>
	struct resolve
	{
		using tuple = decltype(to_tuple(std::declval<T>()));
		using typelist = EEC::MPL::Rename<EEC::MPL::TypeList, tuple>;
	};

	template<typename T>
	using to_member_list_t = typename resolve<T>::typelist;

	template<typename T>
	using to_tuple_t = typename resolve<T>::tuple;


	template<size_t i>
	struct index { static constexpr auto value = i; };

	template<class T, size_t _ = 0>
	struct to_index
	{
		using type = TypeList<>;
	};

	template<typename H, size_t _, typename ...T>
	struct to_index <TypeList<H, T...>, _>
	{
		using type = Concat<TypeList<index<_>>, typename to_index<TypeList<T...>, _ + 1>::type>;
	};

	template<class T, class S, class TL>
	struct replace_all
	{
		using type = TypeList<>;
	};

	template<class T, class S, class ...Ts>
	struct replace_all <T, S, TypeList<T, Ts...>>
	{
		using type = Concat<TypeList<S>, typename replace_all<T, S, TypeList<Ts...>>::type>;
	};

	template<class T, class S, class V, class ...Ts>
	struct replace_all <T, S, TypeList<V, Ts...>>
	{
		using type = Concat<TypeList<V>, typename replace_all<T, S, TypeList<Ts...>>::type>;
	};
}
