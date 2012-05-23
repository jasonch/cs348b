class CreateLayers < ActiveRecord::Migration
  def change
    create_table :layers do |t|
      t.integer :revision_id
      t.string :filepath
      t.integer :zorder
      t.string :name
      t.timestamps
    end
  end
end
