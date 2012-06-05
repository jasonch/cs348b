class CreateUsers < ActiveRecord::Migration
  def change
    create_table :users do |t|
      t.string :login
      t.integer :cur_revision
      t.timestamps
    end
    add_column :revisions, :user_id, :integer
  end
end
