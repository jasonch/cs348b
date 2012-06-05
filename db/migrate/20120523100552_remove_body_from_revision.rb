class RemoveBodyFromRevision < ActiveRecord::Migration
  def up
    remove_column :revisions, :body
      end

  def down
    add_column :revisions, :body, :text
  end
end
