function ImageRect(url) {
  var obj = this; // variable for the closure
  this.url = url;
  this.img = new Image();
  this.img.onload = function() {
          obj.placeImage();
          this.onmousedown = function(event) {
                  obj.startDrag(event);
          }

  };
  this.img.onerror = function() {
    alert('Error loading image');
  };

  this.img.src = url;

}

ImageRect.prototype.placeImage = function () {
  $(this.img).attr('id', 'floatingImage');
  $(this.img).appendTo($('#canvasWrapper'))
          .offset($(myCanvas()).offset());
}

ImageRect.prototype.startDrag = function (event) {
  event.preventDefault();
  var obj = this;
  this.img.onmousemove = function (event) {
    obj.drag(event);
  }
  this.img.onmouseup = function () {
    obj.finalize();
  }
}

ImageRect.prototype.drag = function (event) {
  $(this.img).offset({top: event.pageY - this.img.height/2, left: event.pageX - this.img.width/2});
}

ImageRect.prototype.finalize = function () {
  this.img.onmousemove = EMPTYFUNC;

  if (confirm("Place image? \nOnce an image is placed you cannot move it anymore!")) {
    $(this.img).unbind("mousemove");
    // note +6 to account for border and padding
    myCanvasContext().drawImage(this.img, parseInt(this.img.style.left) + 6, parseInt(this.img.style.top) + 6);
    $(this.img).remove();
  }
}

