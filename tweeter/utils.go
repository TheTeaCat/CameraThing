package main

//The format of a response from the recogniser service
type ClassifyResult struct {
	Filename string        `json:"filename"`
	Labels   []LabelResult `json:"labels"`
}

//A label from within a ClassifyResult as returned by the recogniser service
type LabelResult struct {
	Label       string  `json:"label"`
	Probability float32 `json:"probability"`
}
