package main

//Response from the recogniser service
type ClassifyResult struct {
	Filename string        `json:"filename"`
	Labels   []LabelResult `json:"labels"`
}
type LabelResult struct {
	Label       string  `json:"label"`
	Probability float32 `json:"probability"`
}
