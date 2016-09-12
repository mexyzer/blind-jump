//========================================================================//
// Copyright (C) 2016 Evan Bowman                                         //
// Liscensed under GPL 3, see: <http://www.gnu.org/licenses/>.            //
//========================================================================//

#pragma once

#include <SFML/Graphics.hpp>
#include "Framework/framework.hpp"

class Effect : public framework::Object {
protected:
	int64_t timer;
	uint8_t frameIndex;

public:
	Effect(float _xInit, float _yInit) : Object{_xInit, _yInit}, timer{0}, frameIndex{0} {}
};
