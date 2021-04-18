#ifndef __ENGINE_IDENTIFIABLE_H__
#define __ENGINE_IDENTIFIABLE_H__

/*
    IDENTITY_COLLISION_ERRORS = true
        Used to throw a runtime error if an identifiable object is instantiated with an existing ID
*/

namespace Engine {

	class Identifiable {
		static ArbList<Identifiable*> idmap;
		static unsigned int currentID;

		unsigned int id;
	public:
		static bool IDENTITY_COLLISION_ERRORS;

		Identifiable(unsigned int id=0);
		virtual ~Identifiable();

		virtual int type() = 0;

		inline unsigned int getID() { return id; }

		static Identifiable* getByID(unsigned int id);
	};

}

#endif // __ENGINE_IDENTIFIABLE_H__
