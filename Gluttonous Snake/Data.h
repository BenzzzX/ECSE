#pragma once
#include "./ECS/eecs.hpp"

struct CPosition { int x, y; };
struct CVelocity { int x, y; };

enum CollisionResponse
{
	Block, Overlap
};

struct CCollision
{
	CollisionResponse response;
};

struct CAppearance { char v; };

struct CLifeTime { size_t frames; };

struct CSnake
{
	size_t length;
	/*
	should not compile with this
	virtual void test() {}
	CSnake(size_t l) :length(l) {}
	*/
};

struct CInput { char v; };

struct CFoodSpawner
{
	size_t interval;
	size_t remain;
	size_t holdrand;
};

using Components = eecs::ComponentList
<
	CPosition, CVelocity, CAppearance, CCollision, CLifeTime, CSnake, CInput, CFoodSpawner
>;


using BYTE = unsigned char;
using WorldData = std::pair<size_t, std::vector<BYTE>>;
using FoodData = size_t;

struct CacheEvent
{
	size_t frame = 0;
};

struct TickEvent
{
	size_t frame = 0;
};

struct RewindEvent
{
	size_t &frame;
};

//Note: local id should not be serilized !
struct CollisionEvent
{
	CollisionResponse response;
	size_t id1, id2;
};


struct KeyEvent
{
	char key;
	bool isDown;
};

using Events = eecs::EventList
<
	TickEvent, CollisionEvent, KeyEvent, RewindEvent, CacheEvent
>;



/*快照，用于回放*/
class Cache
{
	template<typename ET, int S>
	class ringbuffer
	{
	public:
		typedef ET value_type;

		ringbuffer()
		{
			clear();
		}

		~ringbuffer() {}

		int size()     const { return m_size; }
		int max_size() const { return S; }

		bool empty() const { return m_size == 0; }
		bool full()  const { return m_size == S; }

		value_type& front() { return m_buffer[m_front]; }
		value_type& back() { return m_buffer[m_back]; }

		void clear() {
			m_size = 0;
			m_front = 0;
			m_back = S - 1;
		}

		void push() {
			m_back = (m_back + 1) % S;
			if (size() == S)
				m_front = (m_front + 1) % S;
			else
				m_size++;
		}

		void push(const value_type& x) {
			push();
			back() = x;
		}

		void pop() {
			if (m_size > 0) {
				m_size--;
				m_back = (m_front + m_size - 1) % S;
			}
		}
	protected:
		std::vector<value_type> m_buffer{ S };

		int m_size;

		int m_front;
		int m_back;
	};
public:
	using TCacheData = std::vector<BYTE>;
	using TFrameCache = std::pair<size_t, TCacheData>;
	ringbuffer<TFrameCache, 10000> cache;
};
/*场景信息，用于碰撞检测*/
class SceneData
{

	using EntityID = size_t;

	std::vector<std::vector<EntityID>> content;
	auto& access(size_t x, size_t y) { return content[y*width + x]; }
public:
	auto& get(size_t x, size_t y)
	{
		return access(x, y);
	}
	void clr()
	{
		for (int i = 0; i < height*width; i++)
			content[i].clear();
	}
	void add(size_t x, size_t y, EntityID eI)
	{
		access(x, y).push_back(eI);
	}
	size_t width, height;
	bool initialized = false;
	SceneData() {}
	SceneData(size_t w, size_t h) :initialized(true), width(w), height(h)
	{
		content.resize(width*height);
	}
};
using Tags = eecs::TagList //定义标记
<
	struct TPlayer, struct TFood, struct TSnakeBody, struct TFood
>;
using Singletons = eecs::SingletonList //定义世界单例
<
	SceneData, Cache
>;

using World = eecs::World //定义世界
<
	Components, Tags, Events, Singletons
>;


template<typename ...Ts>
using EntityTemplate = World::EntityTemplate<Ts...>;

auto snakeTemplate = World::make_template<TPlayer>
(
	CPosition{ 7,4 },
	CAppearance{ 'a' },
	CVelocity{ 1,0 },
	CSnake{ 5u },
	CCollision{ Block },
	CInput{}
);

auto foodTemplate = World::make_template<TFood>
(
	CPosition{ 0,0 },
	CAppearance{'@'},
	CCollision{Overlap}
);
