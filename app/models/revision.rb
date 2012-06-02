class Revision < ActiveRecord::Base
  belongs_to :user
  has_many :revisions
  has_many :layers


  after_create :set_default_layer

  validates_presence_of :title

  def set_default_layer
    layer = Layer.new.default
    layer.revision = self
    layer.datapath="data:null"
    layer.save
  end
end
