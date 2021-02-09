#ifndef __ENGINE_IDENTIFIABLE_H__
#define __ENGINE_IDENTIFIABLE_H__

namespace Engine {

	class Identifiable {
		static ArbList<Identifiable*> idmap;
		static int currentID;

		int id;
	public:
		Identifiable(int id=0);
		virtual ~Identifiable();

		virtual int type() = 0;

		int getID() { return id; }

		static Identifiable* getByID(int id);
	};

}

#endif // __ENGINE_IDENTIFIABLE_H__
