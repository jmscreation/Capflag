#ifndef __ENGINE_H__
#define __ENGINE_H__

#define ENGINE_VERSION 1120

#include <windows.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <atomic>
#include <functional>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"
#include "portaudio.h"

#include "dialog.h"
#include "audio.h"
#include "arbgrid.h"
#include "arblist.h"
#include "sortlist.h"
#include "view.h"
#include "mask.h"
#include "resources.h"
#include "app.h"
#include "inputhandler.h"
#include "active.h"
#include "identifiable.h"
#include "collidable.h"
#include "animation.h"
#include "drawable.h"
#include "gui.h"
#include "gameobject.h"
#include "main.h"

#include "defines.h"

#ifndef ARB_GRID_CELLS
#define ARB_GRID_CELLS 128
#endif

#endif // __INCLUDES_H__
