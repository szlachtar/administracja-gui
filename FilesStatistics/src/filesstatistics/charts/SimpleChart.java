/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package filesstatistics.charts;

import filesstatistics.core.DataStructure;
import filesstatistics.core.StatisticsReader;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.List;
import java.util.Map;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.StandardXYItemRenderer;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

/**
 *
 * @author Rafal
 */
public class SimpleChart {
    private StatisticsReader statisticsReader;
    
    private Map<String, Map<String, Object>> stats;
    
    public SimpleChart(Map<String, Map<String, Object>> stats) {
        this.stats = stats;
    }
    
    public JFreeChart createChart() {
        XYSeries series = new XYSeries("XYGraph");
        DateAxis dateAxis = new DateAxis("Date");
        DateFormat chartFormatter = new SimpleDateFormat("yyyy/MM/dd HH:mm");
        dateAxis.setDateFormatOverride(chartFormatter);
        NumberAxis valueAxis = new NumberAxis();
        
        Map<String, Object> infos = stats.get("a.txt");
        List<DataStructure> dataStructures = (List<DataStructure>)infos.get(StatisticsReader.DATA_STRUCTURE_LIST_KEY);
        Integer counter = 1;
        for(DataStructure ds : dataStructures) {
            if(ds.getOperation().equals("M")) {
                series.add(ds.getDate(), counter);
                counter++;
            }
        }
        
        XYSeriesCollection xyDataset = new XYSeriesCollection(series);
        
        
        StandardXYItemRenderer renderer = new StandardXYItemRenderer(
                StandardXYItemRenderer.SHAPES_AND_LINES, null, null);

        renderer.setShapesFilled(true);

        XYPlot plot = new XYPlot(xyDataset, dateAxis, valueAxis, renderer);

        JFreeChart chart = new JFreeChart("chart", JFreeChart.DEFAULT_TITLE_FONT, plot, false);
        chart.setBackgroundPaint(java.awt.Color.WHITE);

        return chart;
        
    }
}
