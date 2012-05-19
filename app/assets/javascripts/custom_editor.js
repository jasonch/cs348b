
function setLineWidth() {
  var input = "<div class='lightbox'><label for='linewidth'>Line Width:</label><input type='text' id='linewidth' size='1'/></div>";
  $('body').append(input);
  EditorStates.lineWidth = 5;
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
}

function clearCanvas() {
  EditorStates.context.clearRect(0,0, 800,600);
  $('#myCanvas')[0].width = $('#myCanvas')[0].width;
  EditorStates.context.strokeStyle = "#0000ff";
  EditorStates.context.strokeWidth = EditorStates.context.lineWidth;
}

function changeTool(tool) {
  var option;
  for (var i =0; i < EditorStates.options.length; i++) {
    if (EditorStates.options[i].name == tool) {
      option = EditorStates.options[i];
      break;
    }
  }
 
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
    console.log(event.offsetX + ", " + event.offsetY);
    EditorStates.context.beginPath();
    EditorStates.context.moveTo(event.offsetX, event.offsetY);

  }
  function drawScribble(event) {
    console.log(event.X);
    var context = EditorStates.context; 
    context.lineTo(event.offsetX, event.offsetY); 
    context.stroke();

  }
  function finishScribble(event) {
    console.log('save action');
  }

  function initCanvasEvents() {

    for (var i = 0; i < EditorStates.options.length; i++) {
      var option = EditorStates.options[i];
      console.log(option);

      var link = "<a href='#' class='option_" + option.name + "'>" + option.name + "</a>";

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
