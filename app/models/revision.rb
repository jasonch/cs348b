class Revision < ActiveRecord::Base
  has_many :revisions
  has_many :layers

end
