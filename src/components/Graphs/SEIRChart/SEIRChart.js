import React, { Component } from 'react';
import { withTranslation } from 'react-i18next';
import { merge } from '../../../common/utils';

import * as moment from 'moment';

import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
  Brush,
  ResponsiveContainer
} from 'recharts';

import './SEIRChart.scss';

const lineProps = {
  type: 'monotone',
  dot: false,
  strokeWidth: '3'
};

const rkiLineProps = {
  strokeDasharray: '5 5',
  strokeWidth: '2'
};

const lines = [
  {
    dataKey: 'E',
    label: 'parameters.exposed',
    props: {
      stroke: '#ac58e5',
      ...lineProps
    }
  },
  {
    dataKey: 'R',
    label: 'parameters.recovered',
    props: {
      stroke: '#E0488B',
      ...lineProps
    }
  },
  {
    dataKey: 'I',
    label: 'parameters.infected',
    props: {
      stroke: '#9fd0cb',
      ...lineProps
    }
  },
  {
    dataKey: 'S',
    label: 'parameters.sus',
    props: {
      stroke: '#e0d33a',
      ...lineProps
    }
  },
  {
    dataKey: 'AnzahlGenesen',
    label: 'rki.recovered',
    props: {
      stroke: '#7566ff',
      ...lineProps,
      ...rkiLineProps
    }
  },
  {
    dataKey: 'AnzahlFall',
    label: 'rki.infected',
    props: {
      stroke: '#533f82',
      ...lineProps,
      ...rkiLineProps
    }
  },
  {
    dataKey: 'AnzahlTodesfall',
    label: 'rki.deaths',
    props: {
      stroke: '#7a255d',
      ...lineProps,
      ...rkiLineProps
    }
  }
];



const dateFormat = (time) => {
  return moment(time).format('DD. MMM');
};

class SEIRChart extends Component {
  static defaultProps = {
    seir: {
      S: [],
      E: [],
      I: [],
      R: []
    },
    rki: [],
    measures: []
  };

  constructor(props) {
    super(props);
    this.selectBar = this.selectBar.bind(this);
    this.state = {
      lines
    };
  }

  selectBar(event) {
    let updated = [];
    for (let i = 0; i < this.state.lines.length; i++) {
      let line = this.state.lines[i];
      if (line.dataKey !== event.value) {
        updated.push(line);
      } else {
        updated.push({ ...line, inactive: line.inactive === undefined ? true : !line.inactive });
      }
    }
    this.setState({
      lines: updated
    });
  }

  translate(label) {
    const { t } = this.props;
    return t(this.state.lines.find(line => line.dataKey === label).label);
  }

  prepareData() {
    const x = merge(
      JSON.parse(JSON.stringify(this.props.rki)),
      JSON.parse(JSON.stringify(this.props.seir)),
      'date'
    );
    x.sort(function (a, b) {
      return a.date - b.date;
    });

    return x;
  }

  payload() {
    return this.state.lines.map(line => {
      return {
        value: line.dataKey,
        type: 'line',
        id: line.dataKey,
        inactive: line.inactive || false,
        color: line.props.stroke
      };
    });
  }

  render() {
    const data = this.prepareData();
    return (
      <ResponsiveContainer width="100%" height="80%">
        <LineChart
          data={data}
          margin={{ top: 5, right: 30, left: 20, bottom: 5 }}
        >
          <XAxis dataKey="date" tickFormatter={dateFormat} />
          <YAxis />
          <CartesianGrid strokeDasharray="3 3" />
          <Tooltip
            offset={20}
            labelFormatter={dateFormat}
            formatter={(value, name, index) => [value, this.translate(name)]}
            allowEscapeViewBox={{ x: true, y: false }}
            active={true}
            contentStyle={{
              "background-color": 'rgba(255, 255, 255, 0.8)'
            }}
            itemStyle={{
              margin: 0,
              padding: 0
            }}
          />
          {this.state.lines.map(line => {
            return (<Line
              key={line.dataKey}
              dataKey={line.dataKey + (line.inactive ? " " : "")}
              {...line.props}
            />);
          })}
          <Brush dataKey="date" tickFormatter={dateFormat} />
          <Legend
            formatter={this.translate.bind(this)}
            onClick={this.selectBar}
            payload={this.payload()}
            layout='vertical'
            align="right"
            verticalAlign="top"
            wrapperStyle={{
              'padding-left': '1em'
            }}
          />
        </LineChart>
      </ResponsiveContainer>
    );
  }
}

const TranslatedChart = withTranslation()(SEIRChart);

export { TranslatedChart as SEIRChart };