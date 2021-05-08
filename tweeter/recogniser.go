package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"mime/multipart"
	"net/http"
)

//Manages a connection to our recogniser service
type recogniser struct {
	endpoint string
}

//Constructor
func newRecogniser() *recogniser {
	return &recogniser{
		endpoint: "http://imagerec:8080/recognize",
	}
}

func (r *recogniser) recognise(img []byte) ([]LabelResult, error) {
	//Create request body with multipart writer
	var b bytes.Buffer
	multipartWriter := multipart.NewWriter(&b)

	//Create image field in request body
	fileWriter, err := multipartWriter.CreateFormFile("image", "Untitled.jpg")
	if err != nil {
		return nil, errors.New(fmt.Sprintf(
			"Failed to create form file for recogniser request, err: %[1]v",
			err.Error(),
		))
	}

	//Write image to request body
	_, err = io.Copy(fileWriter, bytes.NewReader(img))
	if err != nil {
		return nil, errors.New(fmt.Sprintf(
			"Failed to write image to recogniser request, err: %[1]v",
			err.Error(),
		))
	}

	//Close the multipart writer so terminating boundary set in request
	multipartWriter.Close()

	//Create request with request body
	req, err := http.NewRequest("POST", r.endpoint, &b)

	//Set the content type, will contain the boundary
	req.Header.Set("Content-Type", multipartWriter.FormDataContentType())

	//Make POST request to recogniser
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, err
	}

	//Check status code is correct
	if resp.StatusCode != 200 {
		return nil, errors.New(fmt.Sprintf(
			"Recogniser returned response code: %[1]v",
			resp.StatusCode,
		))
	}

	//Read bytes from response body
	bytes, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, errors.New(fmt.Sprintf(
			"Failed to read bytes from recogniser response, err: %[1]v",
			err.Error(),
		))
	}

	//Unmarshal bytes into recogniserResponse
	data := &ClassifyResult{}
	err = json.Unmarshal(bytes, data)
	if err != nil {
		return nil, errors.New(fmt.Sprintf(
			"Failed to unmarshal response from recogniser, err: %[1]v",
			err.Error(),
		))
	}

	//Return the labels!
	return data.Labels, nil
}
