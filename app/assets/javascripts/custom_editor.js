
function myCanvas(id) {
  if (id == undefined) 
    id = EditorStates.curCanvas;
  return $('#' + id)[0];
}

function myCanvasContext(id) {
  if (id == undefined)
    id = EditorStates.curCanvas;
  return $('#' + id)[0].getContext("2d");

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
  clearCanvas();
  myCanvasContext().drawImage($('#backCanvas')[0], 0, 0);

}

function clearCanvas(id) {
  if (id == undefined)
    id = EditorStates.curCanvas;
  myCanvasContext(id).clearRect(0,0, 800,600);
  myCanvas(id).width = myCanvas(id).width;
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

  $(myCanvas()).unbind('mousedown');
  $(myCanvas()).unbind('mousemove');
  $(myCanvas()).unbind('mouseup');

  if (option) {
      $(myCanvas()).bind("mousedown", function( event ) {
      option.mousedown(event);
      $(myCanvas()).bind("mousemove", function(event) {option.mousemove(event) });
      $(myCanvas()).bind('mouseup', function() {
             option.mouseup(event); 
             $(myCanvas()).unbind('mousemove');
      });
    });

  }

}

  function startScribble(event) {
    myCanvas().changed = true;
    backCanvasContext().drawImage(myCanvas(), 0, 0);
    myCanvasContext().strokeStyle = EditorStates.stroke;
    myCanvasContext().lineWidth =  EditorStates.strokeWidth;
    myCanvasContext().beginPath();
    myCanvasContext().moveTo(event.offsetX, event.offsetY);

  }
  function drawScribble(event) {
    var context = myCanvasContext(); 
    context.lineTo(event.offsetX, event.offsetY); 
    context.stroke();

  }
  function finishScribble(event) {
  }

  function initCanvasEvents() {

    for (var i = 0; i < EditorStates.options.length; i++) {
      var option = EditorStates.options[i];

      var link = "<a href='#' class='option_" + option.name + "'>" + option.title + "</a>";

      $('#optionsPane').append(link);
      $('#optionsPane').find("a.option_"+option.name).click(option.onclick);
    }
  }


function commit() {
  // copy all layers onto backCanvas to create thumbnail
  console.log('commit');
  $('#commitBtn')[0].disabled = 'disabled';

  clearCanvas('backCanvas');

  var layers = $('.canvas'), layerArr = [];
  
  for (var i = 0; i < layers.length; i++) {
    myCanvasContext('backCanvas').drawImage(myCanvas(layers[i].id), 0, 0);
  }

  for (var i = 0; i < layers.length; i++) {
    if (true) { // layers[i].changed, later to implement image file sharing
      var url = myCanvas(layers[i].id).toDataURL('image/png');
      var url = url.replace(/^data:image\/(png|jpg);base64,/,"");

      layerArr.push({id: layers[i].id, zorder: layers[i].style.zIndex, name: "Layer "+layers[i].style.zIndex, filepath: url});
    } 
  }

  if (layerArr.length == 0) return; // no changes made

  var title = $('#revTitle').val(); 
  $.post('/editor/commit', { layers: JSON.stringify(layerArr), title: title}, function(data) {
    console.log(data);
    $('#commitBtn')[0].disabled = '';
  });

  clearCanvas('backCanvas');
}


function showImageURLBox(event) {
  var link = prompt("Image URL: ");
  if (link == null) return;
  new ImageRect(link);

}

function showLineWidthBox (event) {
  var lineWidth = prompt("Enter width in pixels: ", myCanvasContext().lineWidth);
  if (lineWidth == null) return;
  if (isNaN(parseInt(lineWidth))) alert("Invalid number!");
  else {
    EditorStates.strokeWidth = parseInt(lineWidth); 
  }
}

function loadImageIntoLayer(filepath, canvasId) {
  var image = new Image();
  image.onload = function () {
    myCanvas(canvasId).changed = false;
    myCanvas(canvasId).original_file = filepath;
    myCanvasContext(canvasId).drawImage(image, 0, 0);
  };
  image.src = window.location.origin + filepath; 

}



function showCopyLayerDialog() {
  var dia = new CopyLayerDialog(document.body);
}
