#pragma once
#include "ECS\eecs.hpp"

#include <Windows.h>

using BYTE = unsigned char;
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


struct CUniqueID
{
	GUID uid;
};

namespace std
{
	template<> struct hash<GUID>
	{
		size_t operator()(const GUID& guid) const noexcept
		{
			static_assert(sizeof(_GUID) == 128 / CHAR_BIT, "GUID");
			const std::uint64_t* p = reinterpret_cast<const std::uint64_t*>(&guid);
			std::hash<std::uint64_t> hash;
			return hash(p[0]) ^ hash(p[1]);
		}
	};
}

struct UniqueMap
{
	std::unordered_map<GUID, size_t> map;
#ifdef NDEBUG
#else
	bool enabled = false;
#endif
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


enum CollisionResponse
{
	Block, Overlap
};

struct CCollision
{
	CollisionResponse response;
};


struct CollisionEvent
{
	CollisionResponse response;
	size_t id1, id2;
};

struct CAppearance { char v; };

struct CLifeTime { size_t frames; };

struct KeyEvent
{
	char key;
	bool isDown;
};

struct CPosition { int x, y; };
struct CVelocity { int x, y; };