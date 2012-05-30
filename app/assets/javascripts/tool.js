//
//= require_self 
//= require_tree ./tools/
//

function Tool(name, title, canvasCallback) {
  this.name = name;
  this.title = title;
  this.lineWidth = 1;
  this.lineColor = "#000";
  this.canvas = canvasCallback;
}

Tool.prototype.mouseDown = function(){};
Tool.prototype.mouseMove = function(){};
Tool.prototype.mouseUp = function(){};

Tool.prototype.canvasContext = function() {
  return this.canvas().getContext("2d");
}

Tool.prototype.setColor = function(color) {
  this.lineColor = color;
  this.canvasContext().stroke = color;
}
Tool.prototype.setLineWidth = function(width) {
  this.lineWidth = width;
  this.canvasContext().lineWidth = width;
}

