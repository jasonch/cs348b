class CreateLayers < ActiveRecord::Migration
  def change
    create_table :layers do |t|
      t.integer :revision_id
      t.string :filepath
      t.integer :zorder
      t.string :name
      t.timestamps
    end
    
    Revision.find(:all).each do |r|
      layer = Layer.new
      layer.revision = r
      layer.filepath = r.filepath
      layer.zorder = 1
      layer.name = "layer 1"
      layer.save
    end
  end
end
