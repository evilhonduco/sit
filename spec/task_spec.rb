#!/usr/bin/env ruby -w
# encoding: UTF-8
require "rubygems"
require "bundler/setup"
require "rspec"
require "json"
require "rr"
require File.dirname(__FILE__) + "/../sit"
include Sit
include FileUtils

def abs(path)
  File.expand_path(File.dirname(__FILE__) + "/#{path}")
end

describe "Task" do
  before do
    @engine = Engine.new(Parser.new_json, 1_000_000, false)
    cp(abs("sample.json"), abs("tmp.json"))
    @task = Task.new_tail(@engine, abs("tmp.json"))
  end
  
	it "should be tailing the file" do
	  initial = @engine.last_document_id
	  initial.should > 0
	  File.open(abs("tmp.json"), "a") {|f| f.puts(File.read(abs("sample.json")))}
	  @engine.last_document_id.should > initial
	end
end