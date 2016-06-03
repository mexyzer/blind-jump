//
//  musicController.cpp
//  Blind Jump
//
//  Created by Evan Bowman on 1/28/16.
//  Copyright © 2016 Evan Bowman. All rights reserved.
//

#include "soundController.hpp"
#include "ResourcePath.hpp"
#include <iostream>
#include <cmath>
#include "SFML/Graphics.hpp"
#include <iostream>

SoundController::SoundController() {
	
	// Load the soundtrack and play the first song
	soundtrack[0].openFromFile(resourcePath() + "julia-florida.ogg"); /* Composed by Augustin Barrios Mangore, performed by Edson Lopez */
	soundtrack[0].setLoop(true);
	//soundtrack[1].openFromFile(resourcePath() + "deep_blue_sea_blues.ogg");
	//soundtrack[2].openFromFile(resourcePath() + "mister_moon_man.ogg");
	
	// Load effects
	stepEffects[0].loadFromFile(resourcePath() + "step.wav");
	stepEffects[1].loadFromFile(resourcePath() + "grassStep.ogg");
	laserSounds[0].loadFromFile(resourcePath() + "laser.wav");
}

void SoundController::playMusic(unsigned char musicIndex) {
	/*for (int i = 0; i < SOUNDTRACK_LENGTH; i++) {
		// Stop all the other songs
		soundtrack[i].stop();
	}
	// Play the selected one
	soundtrack[musicIndex].play();*/
}

void SoundController::playEffect(SoundController::Effect index) {
	sf::Sound sound;
	switch (index) {
		case SoundController::Effect::step:
			sounds[0].setBuffer(stepEffects[0]);
			sounds[0].setVolume(40);
			sounds[0].play();
			break;
			
		case SoundController::Effect::gunshot:
			sounds[1].setBuffer(laserSounds[0]);
			sounds[1].setVolume(80);
			sounds[1].play();
			break;
			
		case SoundController::Effect::coin:
			//sounds[2].setBuffer(<#const sf::SoundBuffer &buffer#>);
			break;
			
		default:
			std::cerr << "Unexpected effect index in playEffect" << std::endl;
			break;
	}
}