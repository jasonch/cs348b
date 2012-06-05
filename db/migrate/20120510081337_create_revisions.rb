class CreateRevisions < ActiveRecord::Migration
  def change
    create_table :revisions do |t|
      t.text :body
      t.string :title
      t.timestamps
    end
  end
end
