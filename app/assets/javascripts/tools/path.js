function Path(canvasCallback) {
  $('#pathCurPoint').remove();
  $('#pathLastPoint').remove();
  this.canvas = canvasCallback;
  this.firstPoint = true;
  this.canvasContext().beginPath();
  //this.curPoint = $("<div>").attr('id','pathCurPoint').width(10).height(10).css("background-color", "gray").css("position", "absolute").css('z-index', 1000).appendTo($("body")).hide();
  this.lastPoint = $("<div>").attr('id','pathLastPoint')
          .width(10)
          .height(10)
          .css("background-color", "black")
          .css("position", "absolute")
          .css('z-index', 1000)
          .appendTo($("body"))
          .offset({top: 0, left: 0})
          .hide()[0];

  $(this.canvas()).css('cursor', 'crosshair');
}

Path.prototype = new Tool("path", "Path");


Path.prototype.mouseDown = function(event) {
    event.preventDefault();
    this.canvas().changed = true;

    // note this is a hack because somehow the point is 20px lower than it should be, and is only consistent without jquery
    this.lastPoint.style.top = (event.pageY - 25) + "px";
    this.lastPoint.style.left = (event.pageX - 5) + "px";

    $(this.lastPoint).show();
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

