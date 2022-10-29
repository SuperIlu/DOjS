/*
licensed under MIT license or Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0).

MIT License

Copyright (c) 2022 Melanie Eilert <melly_maeh@meilert.net> and Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

This is a minimal text adventure implemented in p5js.
See https://superilu.itch.io/spacebutton for its itch.io page and 
https://openprocessing.org/sketch/1559912 for an online version including
screen reader support for visually impaired.

This games was submitted to August 2022 DOS Game Jam at https://itch.io/jam/dos-games-august-jam
*/

if (navigator.appName === "DOjS") {
	Include("p5");
	in_browser = false;
	dojsOffset = 4;
} else {
	in_browser = true;
	dojsOffset = 0;
}

var SPACING = 10; // spacing between elements
var TEXT_SIZE = 16; // text display size
var TEXT_SPACING = 2; // spacing between two lines of text
var TEXT_COLOR = "#35FF02"; // text color
var LINE_DELAY = 200; // time between displaying two lines of text
//var COMMAND_DELAY = 1200; // delay for switching between two commands
var COMMAND_DELAY = 900; // delay for switching between two commands
var COMMAND_SEPARATOR = " - "; // text separating two commands in the actions box
var BORDER_COLOR = 255; // color of the borders of the boxes
var BORDER_SIZE = 3; // width of the border
var ACTION_BOX_SIZE = 2; // border with of highlighted text in the actions box
var ACTION_HIGHLIGHT = "#ff3502"; // highlight color for the current action in the actions box

var CMD_CLEAR = "<BACK>"; // text for 'back' in the actions list
var CMD_ENTER = "<ENTER>"; // text for 'enter' in the actions box
var CMD_WITH = "WITH"; // text for 'use with' in the actions box

var CMD_INVENTORY = "INVENTORY"; // inventory command
var CMD_USE = "USE"; // use command
var CMD_LOOK = "LOOK"; // look command
var CMD_TAKE = "TAKE"; // take command
var CMD_EAST = "EAST"; // go east command
var CMD_WEST = "WEST"; // go west command
var CMD_NORTH = "NORTH"; // go north command
var CMD_SOUTH = "SOUTH"; // go south command

var TXT_NOTHING = ["You see nothing special"]; // default text for 'nothing to see'
var TXT_CANTGO = ["You can't go there!"]; // default text for 'you can't go there'
var TXT_WHAT = ["I don't understand you!"]; // default text for errors

var current_room;
var con, actions, cmdline;
var person = {
	__inventory__: [],

	// bedroom
	slippers: false, // indicates if the person is wearing the slippers
	bathrobe: false, // indicates if the person is wearing the bathrobe

	// hallway
	broken_toilet: false, // indicates if the person knows the toilet is broken
	work_ready: false,    // indicates if the person is work_ready, influences the ability to put on the space suit/helmet

	// bathroom
	hair_combed: false,   // indicates if the hair was combed
	teeth_brushed: false, // indicates if the teeth were brushed
	toilet_used: false,   // indicates if the toilet was used

	// kitchen
	drank_coffee: false,  // the coffee was consumed
	eaten_toast: false,   // the toast was eaten
	eaten_banana: false,  // the banana was eaten
};

//////
// schlafzimmer
var BEDROOM_ALARMCLOCK = "alarmclock"; // must be turned off in order to get rid of the annoing RING
var BEDROOM_SLIPPERS = "slippers"; // must be worn in order to leave the room
var BEDROOM_BATHROBE = "bathrobe"; // must be worn in order to leave the room
var BEDROOM_LIGHT_SWITCH = "light switch"; // must be truned on in order to see more than the light switch and alarm clock
var BEDROOM_WINDOW = "window"; // red hering, can't be opened

// east: flur
var space_bedroom = {
	text: function (cmd, room) {
		var txt = [];
		if (room.objects.introduction) {
			txt = [];
			if (in_browser) {
				// screen reader support only in browser
				txt = txt.concat([
					"Screen reader support: Press SPACE to confirm a selection.",
					"Press T for the current rooms text. Press R to toggle reading actions.",
					"",
				]);
			}
			txt = txt.concat([
				"Two month ago you've started your new job at a space station.",
				"The first days were very exciting.",
				"Everything was new, everything was different than at your previous jobs.",
				"Meanwhile, you already have your routines and a structured day.",
				"At some time in the morning your alarm clock will ring, you'll get up",
				"and after getting ready you'll start your assigned tasks for the day.",
				"",
				"But some days will break these routines and so will today...",
				"",
			]);
			room.objects.introduction = false;
		}
		txt.push("You are barely awake in your bedroom.");

		if (room.objects.alarmclock) {
			txt.push("The alarmclock is ringing.");
			txt.push("!!! RING !!!");
		}
		if (room.objects.light) {
			txt.push("The lights are on.");
			if (room.objects.alarmclock) {
				txt.push("!!! RING !!!");
			}
			txt.push("There is a door to the east.");
			if (room.objects.alarmclock) {
				txt.push("!!! RING !!!");
			}
		} else {
			txt.push("The room is pitch black beside an illuminated light switch.");
			if (room.objects.alarmclock) {
				txt.push("!!! RING !!!");
			}
		}
		return txt;
	},
	commands: {
		look: function (cmd, room) {
			// looking around only works with light
			if (!room.objects.light) {
				return ["It is to dark, you can't see anything new"];
			} else {
				var txt = [
					"You see your bedroom.",
					"On one wall is a window to the outside.",
				];
				if (room.objects.alarmclock) {
					txt.push("The alarmclock on your bedside table is ringing.");
					txt.push("!!! RING !!!");
				} else {
					txt.push("There is a roughed up alarmclock on your bedside table.");
				}
				if (room.objects.slippers) {
					txt.push("Your slippers are on the floor in front of your bed.");
				}
				if (room.objects.bathrobe) {
					txt.push("Your bathrobe hangs on a hook on the wall.");
				}
				return txt;
			}
		},
		use: function (cmd, room) {
			if (cmd.length === 2 && cmd[1] === BEDROOM_LIGHT_SWITCH) {
				// toggle light
				room.objects.light = !room.objects.light;
				return ["You toggle the light switch"];
			} else if (cmd.length === 2 && cmd[1] === BEDROOM_ALARMCLOCK) {
				// turn off alarm
				room.objects.alarmclock = false;
				return ["You give your alarm clock a hearty smack on the button..."];
			} else if (cmd.length === 2 && cmd[1] === BEDROOM_WINDOW) {
				// open window
				return [
					"Even though you are still very sleepy you realize it is",
					"a VERY BAD idea to air the room in a space station.",
				];
			} else {
				return TXT_WHAT;
			}
		},
		take: function (cmd, room) {
			if (cmd.length === 2 && cmd[1] === BEDROOM_SLIPPERS) {
				person.slippers = true;
				room.objects.slippers = false;
				return ["You put on your slippers"];
			} else if (cmd.length === 2 && cmd[1] === BEDROOM_BATHROBE) {
				person.bathrobe = true;
				room.objects.bathrobe = false;
				return ["You put on the bathrobe"];
			} else {
				return TXT_WHAT;
			}
		},
		walk: function (cmd, room) {
			// is the light on?
			if (!room.objects.light) {
				return ["You can't go anywhere, it is to dark!"];
			} else {
				if (!person.slippers) {
					return ["You refuse to run around the space station with bare feet."];
				}
				if (!person.bathrobe) {
					return [
						"You can't run around the space station stark naked.",
						"What would the neighbours think!?",
					];
				}
				// only door ist EAST to the hallway
				if (cmd[0] === CMD_EAST) {
					if (room.objects.alarmclock) {
						return [
							"Do you really want to annoy your neighbors",
							"by letting the alarm sound indefinitely?"
						];
					} else {
						setNewRoom(space_hallway);
						return ["You walk through the door into the hallway..."];
					}
				} else {
					return TXT_CANTGO;
				}
			}
		},
	},
	take_words: function (cmd, room) {
		var ret = [];
		if (room.objects.light) {
			if (room.objects.slippers) {
				ret.push(BEDROOM_SLIPPERS);
			}
			if (room.objects.bathrobe) {
				ret.push(BEDROOM_BATHROBE);
			}
		}
		return ret;
	},
	use_words: function (cmd, room) {
		var ret = [BEDROOM_LIGHT_SWITCH, BEDROOM_ALARMCLOCK];
		if (room.objects.light) {
			ret.push(BEDROOM_WINDOW);
		}
		return ret;
	},
	objects: {
		introduction: true,
		light: false,
		alarmclock: true,
		slippers: true,
		bathrobe: true,
	},
};

//////
// hallway

var HALLWAY_SPACESUIT = "space suit"; // can only be worn when "work ready". must be worn to exit
var HALLWAY_SPACEHELMET = "space helmet"; // can only be worn when "work ready". must be worn to exit
var HALLWAY_LOCKER = "locker"; // can be opened/closed, contains the toolbox
var HALLWAY_TOOLBOX = "toolbox"; // only visible/takeable when when locker is open, needed to repair the toilet
var HALLWAY_NEWSPAPER = "newspaper"; // needed to use the toilet

// west: bedroom, east: exit, north: bathroom, south: kitchen
var space_hallway = {
	text: function (cmd, room) {
		person.work_ready =
			person.toilet_used && person.hair_combed && person.teeth_brushed &&
			person.drank_coffee && person.eaten_toast && person.eaten_banana;

		return [
			"You are in a Hallway.",
			"There are doors on the north, south, west and on the east.",
		];
	},
	commands: {
		look: function (cmd, room) {
			var txt = ["There is a locker in one corner of the hallway."];
			if (room.objects.locker) {
				if (room.objects.toolbox) {
					txt.push("You can see a toolbox and lots of junk inside the locker.");
				} else {
					txt.push("You can see a lot of junk inside the locker.");
				}
			} else {
				txt.push("The locker is closed.");
			}
			if (room.objects.space_helmet) {
				txt.push("There is a space helmet on the wardrobe.");
			}
			if (room.objects.space_suit) {
				txt.push("There is a space suit on the wardrobe.");
			}
			if (room.objects.newspaper) {
				txt.push(
					"A newspaper is on the floor below the mail slot in the door."
				);
			}
			return txt;
		},
		use: function (cmd, room) {
			if (cmd.length === 2 && cmd[1] === HALLWAY_LOCKER) {
				// open/close locker
				room.objects.locker = !room.objects.locker;
				if (room.objects.locker) {
					return ["You open the locker"];
				} else {
					return ["You close the locker"];
				}
			} else if (cmd.length === 2 && cmd[1] === HALLWAY_NEWSPAPER) {
				return ["You stand in the hallway and flip through the newspaper."];
			} else {
				return TXT_WHAT;
			}
		},
		take: function (cmd, room) {
			if (cmd.length === 2 && cmd[1] === HALLWAY_NEWSPAPER) {
				putInventory(HALLWAY_NEWSPAPER);
				room.objects.newspaper = false;
				return ["You take the newspaper."];
			} else if (cmd.length === 2 && cmd[1] === HALLWAY_SPACEHELMET) {
				if (person.work_ready) {
					person.space_helmet = true;
					room.objects.space_helmet = false;
					return ["You put on the space helmet"];
				} else {
					return [
						"You still need to finish you morning routine before dressing up for work.",
					];
				}
			} else if (cmd.length === 2 && cmd[1] === HALLWAY_SPACESUIT) {
				if (person.work_ready) {
					person.space_suit = true;
					person.bathrobe = false;
					room.objects.space_suit = false;
					return ["You put on the space suit"];
				} else {
					return [
						"You still need to finish you morning routine before dressing up for work.",
					];
				}
			} else if (cmd.length === 2 && cmd[1] === HALLWAY_TOOLBOX) {
				if (person.broken_toilet) {
					putInventory(HALLWAY_TOOLBOX);
					room.objects.toolbox = false;
					return ["That toolbox might be handy to fix the broken toilet."];
				} else {
					return [
						"Most people don't haul toolboxes around when doing their morning routine.",
					];
				}
			} else {
				return TXT_WHAT;
			}
		},
		walk: function (cmd, room) {
			if (cmd[0] === CMD_WEST) {
				setNewRoom(space_bedroom);
				return ["You walk through the door into the bedroom."];
			} else if (cmd[0] === CMD_NORTH) {
				setNewRoom(space_bathroom);
				return ["You walk through the door into the bathroom."];
			} else if (cmd[0] === CMD_SOUTH) {
				if (person.toilet_used) {
					setNewRoom(space_kitchen);
					return ["You walk through the door into the kitchen."];
				} else {
					return [
						"You have VERY urgent business on the bathroom ",
						"before going into the kitchen."
					];
				}
			} else if (cmd[0] === CMD_EAST) {
				if (person.space_suit && person.space_helmet) {
					setNewRoom(space_finish);
					return [
						"You close the visor of the helmet and leave your apartment for work."
					];
				} else {
					return ["You need to put on your work clothes before leaving the apartment."];
				}
			} else {
				return TXT_CANTGO;
			}
		},
	},
	take_words: function (cmd, room) {
		var ret = [];
		if (room.objects.space_helmet) {
			ret.push(HALLWAY_SPACEHELMET);
		}
		if (room.objects.space_suit) {
			ret.push(HALLWAY_SPACESUIT);
		}
		if (room.objects.newspaper) {
			ret.push(HALLWAY_NEWSPAPER);
		}

		if (room.objects.toolbox && room.objects.locker) {
			ret.push(HALLWAY_TOOLBOX);
		}
		return ret;
	},
	use_words: function (cmd, room) {
		return [HALLWAY_LOCKER];
	},
	objects: {
		space_suit: true,
		space_helmet: true,
		locker: false, // locker door is closed when false
		toolbox: true,
		newspaper: true,
	},
};

//////
// bathroom

var BATHROOM_TOILET = "toilet"; // is broken, must be repaired, needs newspaper, must be used in order to get work_ready
var BATHROOM_TOOTHBRUSH = "toothbrush"; // must be used in order to get work_ready
var BATHROOM_TOOTHPASTE = "toothpaste"; // must be used with toothbrush
var BATHROOM_COMB = "comb"; // must be used in order to get work_ready
var BATHROOM_MIRROR = "mirror"; // triggers funny comments depending on the "readiness"

var space_bathroom = {
	text: function (cmd, room) {
		return ["You are in standard space station bathroom."];
	},
	commands: {
		look: function (cmd, room) {
			var ret = [
				"In the corner is a vacuum toilet for space usage.",
				"There is a mirror on the wall.",
			];
			if (room.objects.toothpaste) {
				ret.push("Below the mirror you see a comb, a toothbrush and some toothpaste.");
			} else {
				ret.push("Below the mirror you see a comb and a toothbrush.");
			}
			return ret;
		},
		use: function (cmd, room) {
			if (cmd.length === 2 && cmd[0] === CMD_USE && cmd[1] === BATHROOM_MIRROR) {
				var ret = ["You see yourself in the mirror"];

				if (!person.hair_combed) {
					ret.push(
						"Your hair looks like you participated in a nuclear explosion."
					);
				} else {
					ret.push("Mmmh, what nice hair!");
				}

				if (!person.teeth_brushed) {
					ret.push(
						"Half of yesterdays dinner is still located in your interdental spaces."
					);
				} else {
					ret.push("You are blinded by the glitter of your teeth.");
				}
				return ret;
			} else if (cmd.length === 2 && cmd[1] === HALLWAY_NEWSPAPER) {
				return ["You stand in the bathroom and flip through the newspaper."];
			} else if (cmd.length === 2 && cmd[1] === BATHROOM_TOOTHBRUSH) {
				return [
					"Scrubbing away at your teeth with just the brush would not yield",
					"the wanted result.",
				];
			} else if (cmd.length === 2 && cmd[1] === BATHROOM_COMB) {
				if (person.hair_combed) {
					return ["Every hair on your head is in the exact location it was meant to be.",];
				} else {
					person.hair_combed = true;
					return ["You groom your hair thoroughly like you were Rapunzel."];
				}
			} else if (cmd.length === 2 && cmd[1] === BATHROOM_TOILET) {
				if (room.objects.toilet_repaired) {
					return ["You will need some distraction during your usually long toilet sessions.",];
				} else {
					person.broken_toilet = true;
					return [
						"You try to turn on the vacuum toilet, but there are weird sounds",
						"from the pump. Seems like the toilet is broken!",
					];
				}
			} else if (checkCombination(cmd, BATHROOM_MIRROR, BATHROOM_TOOTHPASTE)) {
				return ["It would be very childish to smear toothpaste on the mirror, wouldn't it?",];
			} else if (checkCombination(cmd, BATHROOM_TOILET, BATHROOM_TOOTHBRUSH)) {
				return ["EEEEEeeeeewwwwwwwwww!"];
			} else if (checkCombination(cmd, BATHROOM_TOILET, HALLWAY_NEWSPAPER)) {
				if (room.objects.ass_cleaned) {
					return ["Even space contains more mater than your bovels right now!"];
				} else {
					if (room.objects.toilet_repaired) {
						if (!room.objects.bovel_movement) {
							room.objects.bovel_movement = true;
							return ["CENSORED"];
						} else {
							room.objects.ass_cleaned = true;
							person.toilet_used = true;
							delInventory(HALLWAY_NEWSPAPER);
							return ["You wipe your ass with the newspaper."];
						}
					} else {
						person.broken_toilet = true;
						return [
							"You try to turn on the vacuum toilet, but there are weird sounds",
							"from the pump. Seems like the toilet is broken!",
						];
					}
				}
			} else if (checkCombination(cmd, BATHROOM_TOILET, HALLWAY_TOOLBOX)) {
				if (room.objects.toilet_repaired) {
					return [
						"The toilet is working again.",
						"Fiddling with it some more will most probably only break it again.",
					];
				} else {
					room.objects.toilet_repaired = true;
					delInventory(HALLWAY_TOOLBOX);
					return [
						"You hammer away at the pump until it is working flawlessly.",
					];
				}
			} else if (checkCombination(cmd, BATHROOM_TOOTHBRUSH, BATHROOM_TOOTHPASTE)) {
				if (person.teeth_brushed) {
					return ["Your teeth won't get whiter than white."];
				} else {
					person.teeth_brushed = true;
					delInventory(BATHROOM_TOOTHPASTE);
					return [
						"You have a minty explosion in your mouth as you brush your teeth.",
					];
				}
			} else {
				return TXT_WHAT;
			}
		},
		take: function (cmd, room) {
			if (cmd.length === 2 && cmd[1] === BATHROOM_TOOTHPASTE) {
				putInventory(BATHROOM_TOOTHPASTE);
				room.objects.toothpaste = false;
				return ["You take the toothpaste."];
			}
		},
		walk: function (cmd, room) {
			if (cmd[0] === CMD_SOUTH) {
				if (room.objects.bovel_movement && !room.objects.ass_cleaned) {
					return [
						"You should clean your bum first before leaving the bathroom!",
					];
				} else {
					setNewRoom(space_hallway);
					return ["You walk through the door into the hallway."];
				}
			} else {
				return TXT_CANTGO;
			}
		},
	},
	take_words: function (cmd, room) {
		if (room.objects.toothpaste) {
			return [BATHROOM_TOOTHPASTE];
		} else {
			return [];
		}
	},
	use_words: function (cmd, room) {
		return [
			BATHROOM_MIRROR,
			BATHROOM_TOOTHBRUSH,
			BATHROOM_COMB,
			BATHROOM_TOILET,
		];
	},
	objects: {
		toothbrush_paste: false,
		toothpaste: true,
		toilet_repaired: false,
		bovel_movement: false,
		ass_cleaned: false,
	},
};

//////
// kitchen

var KITCHEN_EGGLICATOR = "egglicator";
var KITCHEN_TOASTYNATOR = "toastynator";
var KITCHEN_COFFEEMACHINE = "coffee machine";
var KITCHEN_FRUITCHUTE = "fruitchute";

var KITCHEN_PLATE = "plate";
var KITCHEN_PLATE_WITH_TOAST = "plate with toast";
var KITCHEN_PLATE_WITH_TOAST_EGG = "plate with toast and egg";

var KITCHEN_CUP = "cup";
var KITCHEN_CUP_OF_COFFEE = "cup of coffee";

var KITCHEN_BANANA = "banana";

var space_kitchen = {
	text: function (cmd, room) {
		return ["This is a fully automated space station kitchen."];
	},
	commands: {
		look: function (cmd, room) {
			return [
				"On the wall there is a nutrition plan for astronauts.",
				"It reads: 'BREAKFAST - One toast with fried eggs, a coffe with milk",
				"and a banana'. There is a slit on the wall labeled 'toastynator'.",
				"There is a recess labled 'egglicator'. You see a coffee machine.",
				"A pipe with an opening comes down from the ceiling, labeled 'fruitchute'.",
				"A shelf with plates, cups and cutlery is on the wall.",
				"In the north there is the door to the hallway."
			];
		},
		use: function (cmd, room) {
			if (cmd.length === 2 && cmd[1] === KITCHEN_TOASTYNATOR) {
				return ["The toastynator makes a buzzing error sound."];
			} else if (cmd.length === 2 && cmd[1] === KITCHEN_EGGLICATOR) {
				return ["The egglicator makes a cackling error sound."];
			} else if (cmd.length === 2 && cmd[1] === KITCHEN_COFFEEMACHINE) {
				return ["The coffee machine makes a gurgling error sound."];
			} else if (cmd.length === 2 && cmd[1] === KITCHEN_FRUITCHUTE) {
				if (room.objects.banana) {
					putInventory(KITCHEN_BANANA);
					room.objects.banana = false;
					return ["The fruitchute procures a nice, yellow banana."];
				} else {
					return ["The fruitchute has a constipation. I asume from to many bananas."];
				}
			} else if (cmd.length === 2 && cmd[1] === KITCHEN_PLATE_WITH_TOAST) {
				return ["Dry toast without anything would get stuck in your throat."];
			} else if (cmd.length === 2 && cmd[1] === KITCHEN_BANANA) {
				delInventory(KITCHEN_BANANA);
				person.eaten_banana = true;
				return ["You skin and eat the banana. You feel banananized now."];
			} else if (cmd.length === 2 && cmd[1] === KITCHEN_CUP_OF_COFFEE) {
				person.drank_coffee = true;
				delInventory(KITCHEN_CUP_OF_COFFEE);
				return ["You carefully drink the hot coffee and you are wide awake now."];
			} else if (cmd.length === 2 && cmd[1] === KITCHEN_PLATE_WITH_TOAST_EGG) {
				person.eaten_toast = true;
				delInventory(KITCHEN_PLATE_WITH_TOAST_EGG);
				return ["You chew on your tasty toast and egg. Space chickens are the best..."];
			} else if (checkCombination(cmd, KITCHEN_PLATE, KITCHEN_TOASTYNATOR)) {
				room.objects.toast = false;
				delInventory(KITCHEN_PLATE);
				putInventory(KITCHEN_PLATE_WITH_TOAST);
				return ["The toastynator ejects a warm, brown toast onto the plate."];
			} else if (checkCombination(cmd, KITCHEN_TOASTYNATOR, KITCHEN_PLATE_WITH_TOAST)) {
				return ["The toastynator refuses to put toast on toast."];
			} else if (checkCombination(cmd, KITCHEN_PLATE, KITCHEN_EGGLICATOR)) {
				return ["Eggs MUST be placed on toasts, this is required by space law!",];
			} else if (checkCombination(cmd, KITCHEN_PLATE_WITH_TOAST, KITCHEN_EGGLICATOR)) {
				room.objects.egg = false;
				delInventory(KITCHEN_PLATE_WITH_TOAST);
				putInventory(KITCHEN_PLATE_WITH_TOAST_EGG);
				return ["The egglicator egglicates a fried egg onto your toast."];
			} else if (checkCombination(cmd, KITCHEN_PLATE_WITH_TOAST_EGG, KITCHEN_EGGLICATOR)) {
				return ["The egglicator can not egglicate eggs on eggs."];
			} else if (checkCombination(cmd, KITCHEN_CUP_OF_COFFEE, KITCHEN_COFFEEMACHINE)) {
				return [
					"The advanced remote fluid level sensor of the coffee machine",
					"prevents your cup from overflowing to the floor!"
				];
			} else if (checkCombination(cmd, KITCHEN_CUP, KITCHEN_COFFEEMACHINE)) {
				delInventory(KITCHEN_CUP);
				putInventory(KITCHEN_CUP_OF_COFFEE);
				return [
					"The coffee machine hisses and steams to produce a steaming coffee",
					"with creamy space milk."
				];
			} else {
				return TXT_WHAT;
			}
		},
		take: function (cmd, room) {
			if (cmd.length === 2 && cmd[1] === KITCHEN_PLATE) {
				putInventory(KITCHEN_PLATE);
				room.objects.plate = false;
				return ["You take a plate."];
			} else if (cmd.length === 2 && cmd[1] === KITCHEN_CUP) {
				putInventory(KITCHEN_CUP);
				room.objects.cup = false;
				return ["You take a cup."];
			} else {
				return TXT_WHAT;
			}
		},
		walk: function (cmd, room) {
			if (cmd[0] === CMD_NORTH) {
				setNewRoom(space_hallway);
				return ["You walk through the door into the hallway."];
			} else {
				return TXT_CANTGO;
			}
		},
	},
	take_words: function (cmd, room) {
		var ret = [];

		if (room.objects.cup) {
			ret.push(KITCHEN_CUP);
		}

		if (room.objects.plate) {
			ret.push(KITCHEN_PLATE);
		}

		return ret;
	},
	use_words: function (cmd, room) {
		return [
			KITCHEN_EGGLICATOR,
			KITCHEN_TOASTYNATOR,
			KITCHEN_COFFEEMACHINE,
			KITCHEN_FRUITCHUTE,
		];
	},
	objects: {
		cup: true,
		plate: true,
		toast: true,
		egg: true,
		banana: true,
	},
};

//////
// the end
var space_finish = {
	text: function (cmd, room) {
		return [
			"Thank you for playing SpaceButton.",
			"A game (c) 2022 by @melly_maeh and @dec_hl.",
			"Visit us at:",
			"    https://twitter.com/melly_maeh",
			"    https://twitter.com/dec_hl",
			"",
			"Press ESCAPE to exit!"
		];
	},
	commands: {
		look: function (cmd, room) {
			return ["That's all folks, thanks for the fish!"];
		},
		use: function (cmd, room) {
			return ["Press ESCAPE to exit!"];
		},
		take: function (cmd, room) {
			return ["Press ESCAPE to exit!"];
		},
		walk: function (cmd, room) {
			return ["Press ESCAPE to exit!"];
		},
	},
	take_words: function (cmd, room) {
		return [];
	},
	use_words: function (cmd, room) {
		return [];
	},
	objects: {
	},
};


var readActions = false; // true to enable reading the highlighted actions
var readRoom = false; // true to read the current room text
var lastText = ""; // last text that was read by the screenreader

/**
 * p5js setup
 */
function setup() {
	createCanvas(windowWidth, windowHeight);

	con = new Console();
	actions = new Actions(con);
	cmdline = new CommandLine(actions);

	setNewRoom(space_bedroom);

	//setNewRoom(space_kitchen);
	// putInventory(HALLWAY_TOOLBOX);
	// putInventory(HALLWAY_NEWSPAPER);

	// 	person.toilet_used = true;
	// 	person.hair_combed = true;
	// 	person.teeth_brushed = true;
	// 	person.drank_coffee = true;
	// 	person.eaten_toast = true;
	// 	person.eaten_banana = true;

	// 	setNewRoom(space_hallway);
	// print("person.toilet_used=" + person.toilet_used);
	// print("person.hair_combed=" + person.hair_combed);
	// print("person.teeth_brushed=" + person.teeth_brushed);
	// print("person.drank_coffee=" + person.drank_coffee);
	// print("person.eaten_toast=" + person.eaten_toast);
	// print("person.eaten_banana=" + person.eaten_banana);
}

/**
 * p5js draw
 */
function draw() {
	background(32);

	actions.Update();

	con.Draw();
	actions.Draw();
	cmdline.Draw();
}

/**
 * p5js mouse clicked
 */
function mouseClicked() {
	actions.Clicked();
	return false;
}

/**
 * p5js key typed
 */
function keyTyped() {
	actions.Clicked();
	return false;
}

function keyPressed() {
	if (keyCode === 84) {
		// "T"
		print("readRoom");
		readRoom = true;
		readActions = false;
		lastText = "";
		return false;
	} else if (keyCode === 82) {
		// "R"
		print("toggleActions");
		readActions = !readActions;
		return false;
	}
}

function checkCombination(cmd, it1, it2) {
	return (cmd.length === 4 && cmd[1] === it1 && cmd[3] === it2) || (cmd.length === 4 && cmd[1] === it2 && cmd[3] === it1);

}

/**
 * put item in inventory
 * @param {string} item the item
 */
function putInventory(item) {
	person.__inventory__.push(item);
}

/**
 * check for item in inventory
 * @param {string} item the item
 * @returns true if in inventory, else false
 */
function hasInventory(item) {
	return person.__inventory__.indexOf(item) > -1;
}

/**
 * remove item from inventory
 * @param {string} item the item
 */
function delInventory(item) {
	var index = person.__inventory__.indexOf(item);
	if (index > -1) {
		person.__inventory__.splice(index, 1);
	}
}

/**
 * switch szene to new room
 * @param {object} room the new room
 */
function setNewRoom(room) {
	current = room;
	updateCurrentActions();
	con.SetText(current.text([], current));
}

/**
 * update the actions that are displayed in the actions box
 */
function updateCurrentActions() {
	var cmd = cmdline.Get();
	if (cmd.length < 1) {
		var base = [CMD_LOOK];

		if (current.take_words([], current).length > 0) {
			base.push(CMD_TAKE);
		}

		if (current.use_words([], current).length > 0) {
			base.push(CMD_USE);
		}

		base.push(CMD_INVENTORY);
		base.push(CMD_NORTH);
		base.push(CMD_SOUTH);
		base.push(CMD_EAST);
		base.push(CMD_WEST);

		actions.SetActions(base);
	} else if (cmd.length == 1) {
		if (cmd[0] === CMD_USE) {
			var usable = current.use_words(cmd, current).concat(person.__inventory__);
			actions.SetActions(usable);
		} else if (cmd[0] === CMD_TAKE) {
			actions.SetActions(current.take_words(cmd, current));
		} else {
			actions.SetActions([]);
		}
	} else if (cmd.length == 2 && cmd[0] === CMD_USE) {
		actions.SetActions([CMD_WITH]);
	} else if (cmd.length == 3 && cmd[0] === CMD_USE) {
		var usable = current.use_words(cmd, current).concat(person.__inventory__);
		actions.SetActions(usable);
	} else {
		actions.SetActions([]);
	}
}

//////
// action list on screen
function Actions(con) {
	this.x = SPACING;
	this.y = con.y + con.height + SPACING;
	this.width = width - 2 * SPACING;
	this.height =
		height - 4 * SPACING - (height / 4) * 3 - (2 * SPACING + TEXT_SIZE);

	this.actions = [];
	this.draw_actions = [];
	this.current = 0;
	this.current_time = millis();
}
Actions.prototype.SetActions = function (act) {
	this.actions = act;
	this.current = 0;
	this.current_time = millis();
};
Actions.prototype.Draw = function () {
	noFill();
	stroke(BORDER_COLOR);
	strokeWeight(BORDER_SIZE);
	rect(this.x, this.y, this.width, this.height);

	textAlign(LEFT, TOP);
	textSize(TEXT_SIZE);
	var xPos = this.x + SPACING;
	var yPos = this.y + SPACING;
	for (var i = 0; i < this.draw_actions.length; i++) {
		var text_width;

		noStroke();
		if (i > 0) {
			fill(TEXT_COLOR);
			text_width = textWidth(COMMAND_SEPARATOR);
			if (xPos + SPACING + text_width >= this.x + this.width) {
				xPos = this.x + SPACING;
				yPos += TEXT_SIZE + TEXT_SPACING;
			}
			text(COMMAND_SEPARATOR, xPos, yPos);
			xPos += text_width;
		}

		if (i == this.current) {
			fill(ACTION_HIGHLIGHT);
		} else {
			fill(TEXT_COLOR);
		}

		text_width = textWidth(this.draw_actions[i]);
		if (xPos + SPACING + text_width >= this.x + this.width) {
			xPos = this.x + SPACING;
			yPos += TEXT_SIZE + TEXT_SPACING;
		}
		text(this.draw_actions[i], xPos, yPos);

		if (i == this.current) {
			noFill();
			stroke(ACTION_HIGHLIGHT);
			strokeWeight(ACTION_BOX_SIZE);
			rect(xPos - 1, yPos - 1 - dojsOffset, text_width + 2, TEXT_SIZE + 2);
		}

		xPos += text_width;
	}
};
Actions.prototype.Update = function () {
	this.draw_actions = [];
	for (i = 0; i < this.actions.length; i++) {
		this.draw_actions.push(this.actions[i]);
	}

	if (!cmdline.IsEmpty()) {
		this.draw_actions.push(CMD_CLEAR);
		this.draw_actions.push(CMD_ENTER);
	}

	if (this.current_time + COMMAND_DELAY < millis()) {
		this.current = (this.current + 1) % this.draw_actions.length;
		this.current_time = millis();

		if (readActions) {
			srSpeak(this.draw_actions[this.current]);
		}
	}
};
Actions.prototype.Clicked = function () {
	if (this.draw_actions[this.current] === CMD_CLEAR) {
		cmdline.Del();
		if (cmdline.IsEmpty()) {
			this.current = 0;
		}
	} else if (this.draw_actions[this.current] === CMD_ENTER) {
		cmdline.Run();
	} else {
		cmdline.Add(this.draw_actions[this.current]);
	}
};

//////
// current command
function CommandLine(act) {
	this.x = SPACING;
	this.y = act.y + act.height + SPACING;
	this.width = width - 2 * SPACING;
	this.height = 2 * SPACING + TEXT_SIZE;

	this.cmd = [];
}
CommandLine.prototype.Draw = function () {
	noFill();
	stroke(BORDER_COLOR);
	strokeWeight(BORDER_SIZE);
	rect(this.x, this.y, this.width, this.height);

	var cmd_txt = "> ";
	for (var i = 0; i < this.cmd.length; i++) {
		cmd_txt += this.cmd[i] + " ";
	}

	var xPos = this.x + SPACING;
	var yPos = this.y + SPACING;
	noStroke();
	fill(TEXT_COLOR);
	text(cmd_txt, xPos, yPos);
};
CommandLine.prototype.Add = function (w) {
	this.cmd.push(w);
	updateCurrentActions();
};
CommandLine.prototype.Del = function () {
	this.cmd.pop();
	updateCurrentActions();
};
CommandLine.prototype.Clear = function () {
	this.cmd = [];
	updateCurrentActions();
};
CommandLine.prototype.IsEmpty = function () {
	return this.cmd.length == 0;
};
CommandLine.prototype.Get = function () {
	return this.cmd;
};
CommandLine.prototype.Run = function () {
	// commando ausfuehren und
	var ret = [];
	if (this.cmd[0] === CMD_LOOK) {
		ret = current.commands.look(this.cmd, current);
	} else if (this.cmd[0] === CMD_INVENTORY) {
		if (person.__inventory__.length > 0) {
			ret.push("Your inventory contains:");
			ret.push(person.__inventory__.join(", "));
		} else {
			ret.push("Your inventory is empty");
		}
	} else if (this.cmd[0] === CMD_USE) {
		if (this.cmd.length >= 2) {
			ret = current.commands.use(this.cmd, current);
		} else {
			ret = ["Use what?"];
		}
	} else if (this.cmd[0] === CMD_TAKE) {
		if (this.cmd.length >= 2) {
			ret = current.commands.take(this.cmd, current);
		} else {
			ret = ["Take what?"];
		}
	} else {
		ret = current.commands.walk(this.cmd, current);
	}
	con.SetText(current.text([], current));
	con.AddText("Command: ");
	var cmd_txt = "";
	for (var i = 0; i < this.cmd.length; i++) {
		cmd_txt += this.cmd[i] + " ";
	}
	con.AddText("  " + cmd_txt);
	con.AddText("");
	con.AddText("Response: ");
	for (var i = 0; i < ret.length; i++) {
		con.AddText("  " + ret[i]);
	}
	this.Clear();
};

//////
// text console for room info
function Console() {
	this.x = SPACING;
	this.y = SPACING;
	this.width = width - 2 * SPACING;
	this.height = (height / 4) * 3;
	this.xPos = 2 * SPACING;

	this.text = [];
}
Console.prototype.Clear = function () {
	this.text = [];
};
Console.prototype.SetText = function (txt) {
	this.text = [];
	for (var i = 0; i < txt.length; i++) {
		this.text.push(txt[i]);
	}
	this.text.push("");
	this.line = 0;
	readRoom = true;
	readActions = false;
	this.line_time = millis();
};
Console.prototype.AddText = function (txt) {
	this.text.push(txt);
};
Console.prototype.Draw = function () {
	noFill();
	stroke(BORDER_COLOR);
	strokeWeight(BORDER_SIZE);
	rect(this.x, this.y, this.width, this.height);

	if (this.text.length > 0) {
		noStroke();
		fill(TEXT_COLOR);
		textSize(TEXT_SIZE);
		textAlign(LEFT, TOP);
		var yPos = this.x + SPACING;
		for (var i = 0; i < this.text.length && i < this.line; i++) {
			text(this.text[i], this.xPos, yPos);
			yPos += TEXT_SIZE + TEXT_SPACING;
		}

		if (
			this.line < this.text.length &&
			this.line_time + LINE_DELAY < millis()
		) {
			this.line++;
			this.line_time = millis();
		}

		// send text to screenreader when the first line is displayed
		if (readRoom) {
			var txt = "";
			for (var i = 0; i < this.text.length; i++) {
				txt += this.text[i] + "\n";
			}
			srSpeak(txt);
			readRoom = false;
		}
	}
};

/**
 * helper for the screenreader
 * @param {*} text
 */
function srSpeak(text) {
	if (in_browser) {
		if (text != lastText) {
			//print("ToRead: " + text);
			window.speechSynthesis.cancel();
			var msg = new SpeechSynthesisUtterance();
			msg.rate = 1.3;
			msg.lang = "en-US";
			msg.text = text;
			window.speechSynthesis.speak(msg);
			lastText = text;
		}
	}
}
