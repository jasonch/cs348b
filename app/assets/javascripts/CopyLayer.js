function CopyLayerDialog(parentEl) {
  var obj = this;
  this.elem = $('<div></div>').addClass('lightbox')
          .attr('id', 'copyLayerDialog')
          .width(800)
          .height(600)
          .offset({top: 50, left: 100})[0];
  this.parentEl = parentEl;


  $('<input type="button" value="Cancel"></input>')
          .click(function() { $(obj.elem).remove()} )
          .appendTo(this.elem);

  $(this.parentEl).append(this.elem);

  this.chooseRevision();
}

CopyLayerDialog.prototype.chooseRevision = function() {
  var obj = this;
  $('<h3>Select a Revision</h3>').appendTo(this.elem);
  $.ajax({url:'/editor/getAllRevisions', success: function (data) {
    var revs = eval( '(' + data + ')');
    for (var i = 0; i < revs.length; i += 1) {
      var radio = $('<input type="radio"></input>')
         .attr('id', 'rev_'+revs[i].id)
         .attr('name', 'select_revision')
         .val(revs[i].id)
         .appendTo($(obj.elem))
         .click(function(rev) {
           return function() {  obj.chooseLayer(rev); };
         }(revs[i]));  
    
     $("<label for='rev_" + revs[i].id + "'>").html(revs[i].title + " (" + revs[i].id + ") ")
          .appendTo($(obj.elem));
    }
  }});
}

CopyLayerDialog.prototype.chooseLayer = function(rev) {
  var obj = this;
  $('#layerSection').remove();
  var section = $('<div><h3>Select a Layer<h3></div>').attr('id', 'layerSection');
  $.post('/editor/getLayersByRevision', {revId: rev.id}, function(data) {
    var layers = eval('('+data+')');
    obj.layers = layers;
    for (var i = 0; i < layers.length; i += 1) {
      var radio = $('<input type="radio"></input>')
          .attr('id', 'radio_layer_'+layers[i].id)
          .attr('name', 'select_layer')
          .val(i)
          .appendTo($(section));

      name = layers[i].name == "thumb"? "flattened" : layers[i].name;

      $("<span>").html(name).appendTo($(section));
      $('<img src="'+ layers[i].filepath + '"/>')
          .width(100)
          .appendTo(section);
    }
    $(section).appendTo(obj.elem);
    $('<input type="button" id ="insertLayerBtn" value="Insert"></input>')
          .appendTo(obj.elem)
          .click(function() { obj.submit(); } );
  });

}

CopyLayerDialog.prototype.submit = function() {
  var i = $("input[name=select_layer]:checked").val();
  console.log(i);
  console.log(this.layers); 
  new ImageRect(this.layers[i].filepath);

  $(this.elem).remove();
}
