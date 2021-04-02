/**
 * create an evaluation chain.
 * @class
 */
function EvalChain() {
	this.chain = [];
};
/**
 * add a function to the chain.
 * 
 * @param {function} f a function to be called by the chain.
 */
EvalChain.prototype.Add = function (f) {
	this.chain.push(f);
};
/**
 * execute the next function (if any).
 * 
 * @returns {boolean} true if there were any functions to execute, else false.
 */
EvalChain.prototype.Step = function () {
	if (this.chain.length > 0) {
		var next = this.chain.shift();
		next();
		return true;
	} else {
		return false;
	}
};

exports.EvalChain = EvalChain;
