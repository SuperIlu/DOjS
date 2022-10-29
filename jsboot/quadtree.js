/*
	The MIT License

	Copyright (c) 2011 Mike Chambers

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/


/****************** QuadTree ****************/

/**
* A QuadTree implementation in JavaScript, a 2d spatial subdivision algorithm.
* @see http://www.mikechambers.com/blog/2011/03/21/javascript-quadtree-implementation/
* @class
* @param {Object} An object representing the bounds of the top level of the QuadTree. The object 
* should contain the following properties : x, y, width, height
* @param {Boolean} pointQuad Whether the QuadTree will contain points (true), or items with bounds 
* (width / height)(false). Default value is false.
* @param {Number} maxDepth The maximum number of levels that the quadtree will create. Default is 4.
* @param {Number} maxChildren The maximum number of children that a node can contain before it is split into sub-nodes.
**/
function QuadTree(bounds, pointQuad, maxDepth, maxChildren) {
	var node;
	if (pointQuad) {

		node = new Node(bounds, 0, maxDepth, maxChildren);
	} else {
		node = new BoundsNode(bounds, 0, maxDepth, maxChildren);
	}

	this.root = node;
};

/**
* The root node of the QuadTree which covers the entire area being segmented.
* @property root
* @typedef {object} Node
**/
QuadTree.prototype.root = null;


/**
* Inserts an item into the QuadTree.
* @param {Object|Array} item The item or Array of items to be inserted into the QuadTree. The item should expose x, y 
* properties that represents its position in 2D space.
**/
QuadTree.prototype.insert = function (item) {
	if (item instanceof Array) {
		var len = item.length;

		var i;
		for (i = 0; i < len; i++) {
			this.root.insert(item[i]);
		}
	} else {
		this.root.insert(item);
	}
};

/**
* Clears all nodes and children from the QuadTree
**/
QuadTree.prototype.clear = function () {
	this.root.clear();
};

/**
* Retrieves all items / points in the same node as the specified item / point. If the specified item
* overlaps the bounds of a node, then all children in both nodes will be returned.
* @param {Object} item An object representing a 2D coordinate point (with x, y properties), or a shape
* with dimensions (x, y, width, height) properties.
**/
QuadTree.prototype.retrieve = function (item) {
	//get a copy of the array of items
	var out = this.root.retrieve(item).slice(0);
	return out;
};

/************** Node ********************/


function Node(bounds, depth, maxDepth, maxChildren) {
	this._bounds = bounds;
	this.children = [];
	this.nodes = [];

	if (maxChildren) {
		this._maxChildren = maxChildren;
	}

	if (maxDepth) {
		this._maxDepth = maxDepth;
	}

	if (depth) {
		this._depth = depth;
	}
}

//subnodes
Node.prototype.nodes = null;
Node.prototype._classConstructor = Node;

//children contained directly in the node
Node.prototype.children = null;
Node.prototype._bounds = null;

//read only
Node.prototype._depth = 0;

Node.prototype._maxChildren = 4;
Node.prototype._maxDepth = 4;

Node.TOP_LEFT = 0;
Node.TOP_RIGHT = 1;
Node.BOTTOM_LEFT = 2;
Node.BOTTOM_RIGHT = 3;


Node.prototype.insert = function (item) {
	if (this.nodes.length) {
		var index = this._findIndex(item);

		this.nodes[index].insert(item);

		return;
	}

	this.children.push(item);

	var len = this.children.length;
	if (!(this._depth >= this._maxDepth) &&
		len > this._maxChildren) {

		this.subdivide();

		var i;
		for (i = 0; i < len; i++) {
			this.insert(this.children[i]);
		}

		this.children.length = 0;
	}
};

Node.prototype.retrieve = function (item) {
	if (this.nodes.length) {
		var index = this._findIndex(item);

		return this.nodes[index].retrieve(item);
	}

	return this.children;
};

Node.prototype._findIndex = function (item) {
	var b = this._bounds;
	var left = (item.x > b.x + b.width / 2) ? false : true;
	var top = (item.y > b.y + b.height / 2) ? false : true;

	//top left
	var index = Node.TOP_LEFT;
	if (left) {
		//left side
		if (!top) {
			//bottom left
			index = Node.BOTTOM_LEFT;
		}
	} else {
		//right side
		if (top) {
			//top right
			index = Node.TOP_RIGHT;
		} else {
			//bottom right
			index = Node.BOTTOM_RIGHT;
		}
	}

	return index;
};


Node.prototype.subdivide = function () {
	var depth = this._depth + 1;

	var bx = this._bounds.x;
	var by = this._bounds.y;

	//floor the values
	var b_w_h = (this._bounds.width / 2); //todo: Math.floor?
	var b_h_h = (this._bounds.height / 2);
	var bx_b_w_h = bx + b_w_h;
	var by_b_h_h = by + b_h_h;

	//top left
	this.nodes[Node.TOP_LEFT] = new this._classConstructor({
		x: bx,
		y: by,
		width: b_w_h,
		height: b_h_h
	},
		depth, this._maxDepth, this._maxChildren);

	//top right
	this.nodes[Node.TOP_RIGHT] = new this._classConstructor({
		x: bx_b_w_h,
		y: by,
		width: b_w_h,
		height: b_h_h
	},
		depth, this._maxDepth, this._maxChildren);

	//bottom left
	this.nodes[Node.BOTTOM_LEFT] = new this._classConstructor({
		x: bx,
		y: by_b_h_h,
		width: b_w_h,
		height: b_h_h
	},
		depth, this._maxDepth, this._maxChildren);


	//bottom right
	this.nodes[Node.BOTTOM_RIGHT] = new this._classConstructor({
		x: bx_b_w_h,
		y: by_b_h_h,
		width: b_w_h,
		height: b_h_h
	},
		depth, this._maxDepth, this._maxChildren);
};

Node.prototype.clear = function () {
	this.children.length = 0;

	var len = this.nodes.length;

	var i;
	for (i = 0; i < len; i++) {
		this.nodes[i].clear();
	}

	this.nodes.length = 0;
};


/******************** BoundsQuadTree ****************/

function BoundsNode(bounds, depth, maxChildren, maxDepth) {
	Node.call(this, bounds, depth, maxChildren, maxDepth);
	this._stuckChildren = [];
}

BoundsNode.prototype = new Node();
BoundsNode.prototype._classConstructor = BoundsNode;
BoundsNode.prototype._stuckChildren = null;

//we use this to collect and conctenate items being retrieved. This way
//we dont have to continuously create new Array instances.
//Note, when returned from QuadTree.retrieve, we then copy the array
BoundsNode.prototype._out = [];

BoundsNode.prototype.insert = function (item) {
	if (this.nodes.length) {
		var index = this._findIndex(item);
		var node = this.nodes[index];

		//todo: make _bounds bounds
		if (item.x >= node._bounds.x &&
			item.x + item.width <= node._bounds.x + node._bounds.width &&
			item.y >= node._bounds.y &&
			item.y + item.height <= node._bounds.y + node._bounds.height) {

			this.nodes[index].insert(item);

		} else {
			this._stuckChildren.push(item);
		}

		return;
	}

	this.children.push(item);

	var len = this.children.length;

	if (!(this._depth >= this._maxDepth) &&
		len > this._maxChildren) {

		this.subdivide();

		var i;
		for (i = 0; i < len; i++) {
			this.insert(this.children[i]);
		}

		this.children.length = 0;
	}
};

BoundsNode.prototype.getChildren = function () {
	return this.children.concat(this._stuckChildren);
};

BoundsNode.prototype.retrieve = function (item) {
	var out = this._out;
	out.length = 0;
	if (this.nodes.length) {
		var index = this._findIndex(item);
		var node = this.nodes[index];

		if (item.x >= node._bounds.x &&
			item.x + item.width <= node._bounds.x + node._bounds.width &&
			item.y >= node._bounds.y &&
			item.y + item.height <= node._bounds.y + node._bounds.height) {

			out.push.apply(out, this.nodes[index].retrieve(item));
		} else {
			//Part of the item are overlapping multiple child nodes. For each of the overlapping nodes, return all containing objects.

			if (item.x <= this.nodes[Node.TOP_RIGHT]._bounds.x) {
				if (item.y <= this.nodes[Node.BOTTOM_LEFT]._bounds.y) {
					out.push.apply(out, this.nodes[Node.TOP_LEFT].getAllContent());
				}

				if (item.y + item.height > this.nodes[Node.BOTTOM_LEFT]._bounds.y) {
					out.push.apply(out, this.nodes[Node.BOTTOM_LEFT].getAllContent());
				}
			}

			if (item.x + item.width > this.nodes[Node.TOP_RIGHT]._bounds.x) {//position+width bigger than middle x
				if (item.y <= this.nodes[Node.BOTTOM_RIGHT]._bounds.y) {
					out.push.apply(out, this.nodes[Node.TOP_RIGHT].getAllContent());
				}

				if (item.y + item.height > this.nodes[Node.BOTTOM_RIGHT]._bounds.y) {
					out.push.apply(out, this.nodes[Node.BOTTOM_RIGHT].getAllContent());
				}
			}
		}
	}

	out.push.apply(out, this._stuckChildren);
	out.push.apply(out, this.children);

	return out;
};

//Returns all contents of node.
BoundsNode.prototype.getAllContent = function () {
	var out = this._out;
	if (this.nodes.length) {

		var i;
		for (i = 0; i < this.nodes.length; i++) {
			this.nodes[i].getAllContent();
		}
	}
	out.push.apply(out, this._stuckChildren);
	out.push.apply(out, this.children);
	return out;
};

BoundsNode.prototype.clear = function () {

	this._stuckChildren.length = 0;

	//array
	this.children.length = 0;

	var len = this.nodes.length;

	if (!len) {
		return;
	}

	var i;
	for (i = 0; i < len; i++) {
		this.nodes[i].clear();
	}

	//array
	this.nodes.length = 0;

	//we could call the super clear function but for now, im just going to inline it
	//call the hidden super.clear, and make sure its called with this = this instance
	//Object.getPrototypeOf(BoundsNode.prototype).clear.call(this);
};

// export functions and version
exports.__VERSION__ = 2;
exports.QuadTree = QuadTree;
