function myCanvas() {
  return $('#myCanvas')[0];
}

function myCanvasContext() {
  return $('#myCanvas')[0].getContext("2d");

}

function backCanvasContext() {
  return $('#backCanvas')[0].getContext("2d");
}


function addScribble() {
        $('.wymeditor').wymeditor({
          postInit: function(wym) {
            var button = "<li class='wym_tools_scribble'><a name='scribbleBotton' href='#'>&nbsp;</a></li>";
            jQuery(wym._box)
              .find(wym._options.toolsSelector + wym._options.toolsListSelector)
              .append(button);
            jQuery(wym._box)
            .find('li.wym_tools_scribble a').click(function() {
              console.log("scribble selected");
              return false;
            });
          }
        });
};


function undo() {
  console.log("undo");
  clearCanvas();
  myCanvasContext().drawImage($('#backCanvas')[0], 0, 0);

}

function clearCanvas() {
  myCanvasContext().clearRect(0,0, 800,600);
  $('#myCanvas')[0].width = $('#myCanvas')[0].width;
  myCanvasContext().lineWidth = EditorStates.strokeWidth; 
  myCanvasContext().strokeStyle = EditorStates.stroke; 
}

function changeTool(tool) {
  var option;
  for (var i =0; i < EditorStates.options.length; i++) {
    if (EditorStates.options[i].name == tool) {
      option = EditorStates.options[i];
      break;
    }
  }
  $('#optionsPane a').removeClass('selected');
  $('.option_'+tool).addClass('selected');

  $('#myCanvas').unbind('mousedown');
  $('#myCanvas').unbind('mousemove');
  $('#myCanvas').unbind('mouseup');

  if (option) {
      $('#myCanvas').bind("mousedown", function( event ) {
      option.mousedown(event);
      $('#myCanvas').bind("mousemove", function(event) {option.mousemove(event) });
      $('#myCanvas').bind('mouseup', function() {
             option.mouseup(event); 
             $('#myCanvas').unbind('mousemove');
      });
    });

  }

}

  function startScribble(event) {
    backCanvasContext().drawImage($('#myCanvas')[0], 0, 0);
    console.log(event.offsetX + ", " + event.offsetY);
    myCanvasContext().beginPath();
    myCanvasContext().moveTo(event.offsetX, event.offsetY);

  }
  function drawScribble(event) {
    console.log(event.X);
    var context = myCanvasContext(); 
    context.lineTo(event.offsetX, event.offsetY); 
    context.stroke();

  }
  function finishScribble(event) {
    console.log('save action');
  }

  function initCanvasEvents() {

    for (var i = 0; i < EditorStates.options.length; i++) {
      var option = EditorStates.options[i];

      var link = "<a href='#' class='option_" + option.name + "'>" + option.title + "</a>";

      $('#optionsPane').append(link);
      $('#optionsPane').find("a.option_"+option.name).click(option.onclick);
    }
  }


  function drawSomething() {
           var canvas = document.getElementById("myCanvas");
            var context = canvas.getContext("2d");
 
            // draw cloud
            context.beginPath(); // begin custom shape
            context.moveTo(170, 80);
            context.bezierCurveTo(130, 100, 130, 150, 230, 150);
            context.bezierCurveTo(250, 180, 320, 180, 340, 150);
            context.bezierCurveTo(420, 150, 420, 120, 390, 100);
            context.bezierCurveTo(430, 40, 370, 30, 340, 50);
            context.bezierCurveTo(320, 5, 250, 20, 250, 50);
            context.bezierCurveTo(200, 5, 150, 20, 170, 80);
            context.closePath(); // complete custom shape
            context.lineWidth = 5;
            context.fillStyle = "#8ED6FF";
            context.fill();
            context.strokeStyle = "#0000ff";
            context.stroke();
 
            // save canvas image as data url (png format by default)
            var dataURL = canvas.toDataURL();
 
            // set canvasImg image src to dataURL
            // so it can be saved as an image
            //document.getElementById("canvasImg").src = dataURL;
  }
 
function commit() {
  var url = $('#myCanvas')[0].toDataURL('image/png');
  var url = url.replace(/^data:image\/(png|jpg);base64,/,"");
  var title = $('#revTitle').val(); 
        console.log(url);
  $.post('/editor/commit', { filepath: url, title: title}, function(data) {

    console.log(data);


  });

}


function showImageURLBox(event) {
  var link = prompt("Image URL: ");
  if (link == null) return;
  var newImage = new Image();
      newImage.onload = function () {
          $(this).attr('id', 'floatingImage')
                .bind('mousedown', function(event) {
                    event.preventDefault();
                    $(this).bind('mousemove', function(event) {
                            $(this).offset({top: event.clientY - $(this)[0].height/2, left: event.clientX- $(this)[0].width/2});
                    });
                    $(this).bind('mouseup', function(event) {
                      if (confirm("Place image? \nOnce an image is placed you cannot move it anymore!")) {
                        $(this).unbind("mousemove");
                        myCanvasContext().drawImage(newImage,$(this).offset().left - $("#myCanvas").offset().left, $(this).offset().top - $("#myCanvas").offset().top);
                        $(this).remove();
                      }
                    });

                })
                .appendTo($("#canvasWrapper")) 
                .offset($('#myCanvas').offset());
        };

  newImage.onerror = function() {
    alert('Error loading image');
  };
  newImage.src=link;


}

function showLineWidthBox (event) {
  var lineWidth = prompt("Enter width in pixels: ", myCanvasContext().lineWidth);
  if (lineWidth == null) return;
  if (isNaN(parseInt(lineWidth))) alert("Invalid number!");
  else {
    myCanvasContext().lineWidth =  parseInt(lineWidth);
    EditorStates.strokeWidth = myCanvasContext().lineWidth;
  }
}
