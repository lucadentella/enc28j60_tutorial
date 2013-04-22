<?php
	
	define("LOG_FILE", "./temperatures.csv");
	require_once('jpgraph/jpgraph.php');
	require_once('jpgraph/jpgraph_line.php');
	
	$times = array();
	$values = array();	
	
	$file_lines = file(LOG_FILE, FILE_IGNORE_NEW_LINES|FILE_SKIP_EMPTY_LINES);
	
	foreach($file_lines as $line_num => $line_value) {
		$line_elements = explode(",", $line_value);
		$times[] = date("H:i:s", $line_elements[0]);
		$values[] = $line_elements[1];
	}
	
	$graph = new Graph(800, 400);
	$graph->SetFrame(false);
	$graph->SetScale('intint');
	$graph->SetMarginColor('white');

	$graph->title->Set("My room's temperature");
	
	$graph->xaxis->SetTickLabels($times);

	$graph->yaxis->scale->SetAutoMin(0);
	$graph->yaxis->title->Set("°C");
	
	$graph->ygrid->SetFill($aFlg=true, $aColor1='white', $aColor2='gray9');

	$lineplot = new LinePlot($values);
	$lineplot->SetColor('blue');
	
	$graph->Add($lineplot);
	$graph->Stroke();
?>