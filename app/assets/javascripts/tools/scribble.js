function Scribble(canvasCallback) {
  this.canvas = canvasCallback;

  this.lineWidth = 1;
  this.lineColor = "#000";

  this.mouseIsDown = false;
  $(this.canvas()).css('cursor', 'default');
}

Scribble.prototype = new Tool("scribble", "Scribble");


Scribble.prototype.mouseDown = function(event) {
    this.canvas().changed = true;

    this.canvasContext().strokeStyle = EditorStates.stroke;
    this.canvasContext().lineWidth = EditorStates.strokeWidth;
    this.canvasContext().beginPath();
    this.canvasContext().moveTo(event.offsetX, event.offsetY);
    this.mouseIsDown = true;
}

Scribble.prototype.mouseMove = function(event) {
    if (this.mouseIsDown == false) return;
    this.canvasContext().lineTo(event.offsetX, event.offsetY); 
    this.canvasContext().stroke();
}

Scribble.prototype.mouseUp = function(event) {
  this.mouseIsDown = false;
}

