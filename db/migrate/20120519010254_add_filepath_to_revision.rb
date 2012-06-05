class AddFilepathToRevision < ActiveRecord::Migration
  def change
    add_column :revisions, :filepath, :string
  end
end
