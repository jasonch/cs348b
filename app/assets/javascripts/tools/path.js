function Path(canvasCallback) {
  this.canvas = canvasCallback;
  this.firstPoint = true;
  this.canvasContext().beginPath();
}

Path.prototype = new Tool("path", "Path");


Path.prototype.mouseDown = function(event) {
    this.canvas().changed = true;

    console.log(this);
    this.canvasContext().strokeStyle = EditorStates.stroke;
    this.canvasContext().lineWidth = EditorStates.strokeWidth;

    if (!this.firstPoint) {
      this.canvasContext().lineTo(event.offsetX, event.offsetY); 
      this.canvasContext().stroke();
    }
    this.canvasContext().moveTo(event.offsetX, event.offsetY);
    this.firstPoint = false;
}

Path.prototype.mouseMove = function(event) {
}

Path.prototype.mouseUp = function(event) {
}

