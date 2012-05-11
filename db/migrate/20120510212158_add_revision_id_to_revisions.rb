class AddRevisionIdToRevisions < ActiveRecord::Migration
  def change
    add_column :revisions, :revision_id, :integer
  end
end
