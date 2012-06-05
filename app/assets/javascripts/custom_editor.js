
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
  myCanvasContext(id).clearRect(0,0, 800,600);
  myCanvas(id).width = myCanvas(id).width;
}

function changeTool(tool) {
  $(myCanvas()).unbind("mousedown");
  $(myCanvas()).unbind("mousemove");
  $(myCanvas()).unbind("mouseup");
      
  $(myCanvas()).bind("mousedown", function() {
          backCanvasContext().drawImage(myCanvas(), 0, 0);
          tool.mouseDown(event); });
  $(myCanvas()).bind("mouseup",  function() { tool.mouseUp(event); });
  $(myCanvas()).bind("mousemove", function() { tool.mouseMove(event); });

  $('#optionsPane a').removeClass('selected');
  $('#option_'+tool.name).addClass('selected');
  return ;

}


  function initCanvasEvents() {

    for (var i = 0; i < EditorStates.options.length; i++) {
      var option = EditorStates.options[i];

      var link = "<a href='#' id='option_" + option.name + "' class='option'>" + option.title + "</a>";

      $('#optionsPane').append(link);
      $('#optionsPane').find("a#option_"+option.name).click(option.onclick);
    }
  }


function commit() {
  // copy all layers onto backCanvas to create thumbnail
  console.log('commit');
  $('#commitBtn')[0].disabled = 'disabled';


  var layers = $('.canvas'), layerArr = [];
 
  // draw every layer onto the backcanvas to create thumbnail
  clearCanvas('backCanvas');
  for (var i = 0; i < layers.length; i++) {
    if (layers[i].id == 'backCanvas') continue;
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
    //console.log(data);
    $('#commitBtn')[0].disabled = '';
  });

  //clearCanvas('backCanvas');
}


function showImageURLBox(event) {
  var link = prompt("Image URL: ");
  if (link == null) return;
  new ImageRect(link);

}

function showLineWidthBox (event) {
  var lineWidth = prompt("Enter width in pixels: ", EditorStates.strokeWidth);
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

function viewTimeline(url) {
  if (confirm("Any uncommitted changes will be lost.", "Continue", "Cancel")) {
    window.location.href = url;
  }
}

function showCopyLayerDialog() {
  var dia = new CopyLayerDialog(document.body);
}
